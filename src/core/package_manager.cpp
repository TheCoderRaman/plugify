#include "package_manager.hpp"
#include "module.hpp"
#include "package_manifest.hpp"
#include "plugin.hpp"

#include <miniz.h>
#include <plugify/plugify.hpp>
#include <utils/file_system.hpp>
#include <utils/json.hpp>
#include <utils/strings.hpp>
#if PLUGIFY_DOWNLOADER
#include <utils/http_downloader.hpp>
#include <utils/sha256.hpp>
#endif // PLUGIFY_DOWNLOADER

using namespace plugify;

static std::array<std::pair<std::string_view, std::string_view>, 2> packageTypes {
	std::pair{ "modules", Module::kFileExtension },
	std::pair{ "plugins", Plugin::kFileExtension },
	// Might add more package types in future
};

PackageManager::PackageManager(std::weak_ptr<IPlugify> plugify) : PlugifyContext(std::move(plugify)) {
}

PackageManager::~PackageManager() {
	Terminate();
}

bool PackageManager::Initialize() {
	if (IsInitialized())
		return false;

	auto debugStart = DateTime::Now();

#if PLUGIFY_DOWNLOADER
	_httpDownloader = HTTPDownloader::Create();
#endif // PLUGIFY_DOWNLOADER

	LoadAllPackages();

	_inited = true;

	PL_LOG_DEBUG("PackageManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
	return true;
}

void PackageManager::Terminate() {
	if (!IsInitialized())
		return;

	_localPackages.clear();
	_remotePackages.clear();
	_missedPackages.clear();
	_conflictedPackages.clear();

#if PLUGIFY_DOWNLOADER
	_httpDownloader.reset();
#endif // PLUGIFY_DOWNLOADER

	_inited = false;
}

bool PackageManager::IsInitialized() const {
	return _inited;
}

bool PackageManager::Reload() {
	if (!IsInitialized())
		return false;

	LoadAllPackages();

	return true;
}

void PackageManager::LoadAllPackages() {
	LoadLocalPackages();
#if PLUGIFY_DOWNLOADER
	LoadRemotePackages();
	FindDependencies();
#endif // PLUGIFY_DOWNLOADER
}

template<typename Cnt, typename Pr = std::equal_to<typename Cnt::value_type>>
bool RemoveDuplicates(Cnt& cnt, Pr cmp = Pr()) {
	auto size = std::size(cnt);
	Cnt result;
	result.reserve(size);

	std::copy_if(
		std::make_move_iterator(std::begin(cnt)),
		std::make_move_iterator(std::end(cnt)),
		std::back_inserter(result),
		[&](const typename Cnt::value_type& what) {
			return std::find_if(std::begin(result), std::end(result), [&](const typename Cnt::value_type& existing) {
				return cmp(what, existing);
			}) == std::end(result);
		}
	);

	cnt = std::move(result);
	return std::size(cnt) != size;
}

void RemoveUnsupported(RemotePackage& package) {
	for (auto it = package.versions.begin(); it != package.versions.end(); ) {
		if (!PackageManager::IsSupportsPlatform(it->platforms)) {
			it = package.versions.erase(it);
		} else {
			++it;
		}
	}
}

void ValidateDependencies(const std::string& name, std::vector<std::string>& errors, std::vector<PluginReferenceDescriptor>& dependencies) {
	if (RemoveDuplicates(dependencies)) {
		PL_LOG_WARNING("Package: '{}' has multiple dependencies with same name!", name);
	}

	for (size_t i = 0; i < dependencies.size(); ++i) {
		const auto& dependency = dependencies[i];

		if (dependency.name.empty()) {
			errors.emplace_back(std::format("Missing dependency name at: {}", i));
		}

		if (dependency.requestedVersion.has_value() && *dependency.requestedVersion < 0) {
			errors.emplace_back(std::format("Invalid dependency version at: {}", i));
		}
	}
}

void ValidateParameters(std::vector<std::string>& errors, const Method& method, size_t i) {
	for (const auto& property : method.paramTypes) {
		if (property.type == ValueType::Void) {
			errors.emplace_back(std::format("Parameter cannot be void type at: {}", method.name.empty() ? std::to_string(i) : method.name));
		} else if (property.type == ValueType::Function && property.ref) {
			errors.emplace_back(std::format("Parameter with function type cannot be reference at: {}", method.name.empty() ? std::to_string(i) : method.name));
		}

		if (property.prototype) {
			ValidateParameters(errors, *property.prototype, i);
		}
	}

	if (method.retType.ref) {
		errors.emplace_back("Return cannot be reference");
	}
}

void ValidateMethods(const std::string& name, std::vector<std::string>& errors, std::vector<Method>& methods) {
	if (RemoveDuplicates(methods)) {
		PL_LOG_WARNING("Package: '{}' has multiple method with same name!", name);
	}

	for (size_t i = 0; i < methods.size(); ++i) {
		const auto& method = methods[i];

		if (method.name.empty()) {
			errors.emplace_back(std::format("Missing method name at: {}", i));
		}

		if (method.funcName.empty()) {
			errors.emplace_back(std::format("Missing function name at: {}", method.name.empty() ? std::to_string(i) : method.name));
		}

		if (!method.callConv.empty()) {
#if PLUGIFY_ARCH_ARM
#if PLUGIFY_ARCH_BITS == 64

#elif PLUGIFY_ARCH_BITS == 32
			if (method.callConv != "soft" && method.callConv != "hard")
#endif // PLUGIFY_ARCH_BITS
#else
#if PLUGIFY_ARCH_BITS == 64 && PLUGIFY_PLATFORM_WINDOWS
			if (method.callConv != "vectorcall")
#elif PLUGIFY_ARCH_BITS == 32
			if (method.callConv != "cdecl" && method.callConv != "stdcall" && method.callConv != "fastcall" && method.callConv != "thiscall" && method.callConv != "vectorcall")
#endif // PLUGIFY_ARCH_BITS
#endif // PLUGIFY_ARCH_ARM
			{
				errors.emplace_back(std::format("Invalid calling convention: '{}' at: {}", method.callConv, method.name.empty() ? std::to_string(i) : method.name));
			}
		}

		ValidateParameters(errors, method, i);

		if (method.varIndex != Method::kNoVarArgs && method.varIndex >= method.paramTypes.size()) {
			errors.emplace_back("Invalid variable argument index");
		}
	}
}

void ValidateDirectories(std::vector<std::string>& errors, const std::vector<std::string>& directories) {
	for (size_t i = 0; i < directories.size(); ++i) {
		const auto& directory = directories[i];
		if (directory.empty()) {
			errors.emplace_back(std::format("Missing directory at: {}", i));
		}
	}
}

template<typename T>
std::optional<LocalPackage> GetPackageFromDescriptor(const fs::path& path, const std::string& name) {
	auto json = FileSystem::ReadText(path);
	auto descriptor = glz::read_json<T>(json);
	if (!descriptor.has_value()) {
		PL_LOG_ERROR("Package: '{}' has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
		return {};
	}

	if (!PackageManager::IsSupportsPlatform(descriptor->supportedPlatforms))
		return {};

	std::vector<std::string> errors;

	if (descriptor->fileVersion < 1) {
		errors.emplace_back("Invalid file version");
	}

	if (descriptor->version < 0) {
		errors.emplace_back("Invalid version");
	}

	if (descriptor->friendlyName.empty()) {
		errors.emplace_back("Missing friendly name");
	}

	if (descriptor->resourceDirectories.has_value()) {
		ValidateDirectories(errors, *descriptor->resourceDirectories);
	}

	std::string type;
	if constexpr (std::is_same_v<T, LanguageModuleDescriptor>) {
		if (descriptor->language.empty() || descriptor->language == "plugin") {
			errors.emplace_back("Missing/invalid language name");
		}

		if (descriptor->libraryDirectories.has_value()) {
			ValidateDirectories(errors, *descriptor->libraryDirectories);
		}

		type = descriptor->language;
	} else {
		if (descriptor->entryPoint.empty()) {
			errors.emplace_back("Missing entry point");
		}
		if (descriptor->languageModule.name.empty()) {
			errors.emplace_back("Missing language name");
		}

		ValidateDependencies(name, errors, descriptor->dependencies);
		ValidateMethods(name, errors, descriptor->exportedMethods);

		type = "plugin";
	}

	if (!errors.empty()) {
		std::string error(errors[0]);
		for (auto it = std::next(errors.begin()); it != errors.end(); ++it) {
			std::format_to(std::back_inserter(error), ", {}", *it);
		}
		PL_LOG_ERROR("Package: '{}' has error(s): {}", name, error);
		return {};
	}

	auto version = descriptor->version;
	return { {Package{name, type}, path, version, std::make_unique<T>(std::move(*descriptor))} };
}

void PackageManager::LoadLocalPackages()  {
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	PL_LOG_DEBUG("Loading local packages");

	_localPackages.clear();
	//_localPackages.reserve()

	FileSystem::ReadDirectory(plugify->GetConfig().baseDir, [&](const fs::path& path, int depth) {
		if (depth != 1)
			return;

		auto extension = path.extension().string();
		bool isModule = extension == Module::kFileExtension;
		if (!isModule && extension != Plugin::kFileExtension)
			return;

		auto name = path.filename().replace_extension().string();
		if (name.empty())
			return;

		auto package = isModule ?
				GetPackageFromDescriptor<LanguageModuleDescriptor>(path, name) :
				GetPackageFromDescriptor<PluginDescriptor>(path, name);
		if (!package.has_value())
			return;

		auto it = _localPackages.find(name);
		if (it == _localPackages.end()) {
			_localPackages.emplace(std::move(name), std::move(*package));
		} else {
			auto& existingPackage = std::get<LocalPackage>(*it);

			auto& existingVersion = existingPackage.version;
			if (existingVersion != package->version) {
				PL_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package->version), name, std::min(existingVersion, package->version));

				if (existingVersion < package->version) {
					existingPackage = std::move(*package);
				}
			} else {
				PL_LOG_WARNING("The same version (v{}) of package '{}' exists at '{}' - second location will be ignored.", existingVersion, name, path.string());
			}
		}
	}, 3);
}

#if PLUGIFY_DOWNLOADER

void PackageManager::LoadRemotePackages() {
	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	PL_LOG_DEBUG("Loading remote packages");

	const auto& repositories = plugify->GetConfig().repositories;

	_remotePackages.clear();
	_remotePackages.reserve(repositories.size() + _localPackages.size());

	std::mutex mutex;

	auto fetchManifest = [&](const std::string& url, const std::shared_ptr<Descriptor>& descriptor = nullptr) {
		if (!String::IsValidURL(url)) {
			PL_LOG_WARNING("Tried to fetch a package: '{}' that is not have valid url: \"{}\", aborting",
						   descriptor ? descriptor->friendlyName : "<from config>", url.empty() ? "<empty>" : url);
			return;
		}
		
		_httpDownloader->CreateRequest(url, [&](int32_t statusCode, std::string_view, HTTPDownloader::Request::Data data) {
			if (statusCode == HTTPDownloader::HTTP_STATUS_OK) {
				/*if (contentType != "text/plain" || contentType != "application/json" || contentType != "text/json" || contentType != "text/javascript") {
					PL_LOG_ERROR("Package manifest: '{}' should be in text format to be read correctly", url);
					return;
				}*/

				std::string buffer(data.begin(), data.end());
				auto manifest = glz::read_json<PackageManifest>(buffer);
				if (!manifest.has_value()) {
					PL_LOG_ERROR("Packages manifest from '{}' has JSON parsing error: {}", url, glz::format_error(manifest.error(), buffer));
					return;
				}

				for (auto& [name, package] : manifest->content) {
					if (name.empty() || package.name != name) {
						PL_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", url, name, package.name);
						continue;
					}
					RemoveUnsupported(package);
					if (package.versions.empty()) {
						PL_LOG_ERROR("Package manifest: '{}' has empty version list at '{}'", url, name);
						continue;
					}

					auto it = _remotePackages.find(name);
					if (it == _remotePackages.end()) {
						std::unique_lock<std::mutex> lock(mutex);
						_remotePackages.emplace(name, std::move(package));
					} else {
						auto& existingPackage = std::get<RemotePackage>(*it);

						if (existingPackage == package) {
							std::unique_lock<std::mutex> lock(mutex);
							existingPackage.versions.merge(package.versions);
						} else {
							PL_LOG_WARNING("The package '{}' exists at '{}' - second location will be ignored.", name, url);
						}
					}
				}
			}
		});
	};

	for (const auto& url : repositories) {
		fetchManifest(url);
	}

	for (const auto& [_, package] : _localPackages) {
		fetchManifest(package.descriptor->updateURL, package.descriptor);
	}

	//FetchPackagesListFromAPI(mutex);

	_httpDownloader->WaitForAllRequests();
}

template<typename T>
const T* FindLanguageModule(const std::unordered_map<std::string, T, string_hash, std::equal_to<>>& container, const std::string& name)  {
	for (const auto& [_, package] : container) {
		if (package.type == name) {
			return &package;
		}
	}
	return nullptr;
}

void PackageManager::FindDependencies() {
	_missedPackages.clear();
	_conflictedPackages.clear();

	for (const auto& [_, package] : _localPackages) {
		if (package.type == "plugin") {
			auto pluginDescriptor = std::static_pointer_cast<PluginDescriptor>(package.descriptor);

			const auto& lang = pluginDescriptor->languageModule.name;
			if (!FindLanguageModule(_localPackages, lang)) {
				auto remotePackage = FindLanguageModule(_remotePackages, lang);
				if (remotePackage) {
					auto it = _missedPackages.find(lang);
					if (it == _missedPackages.end()) {
						_missedPackages.emplace(lang, std::pair{ remotePackage, std::nullopt }); // by default prioritizing latest language modules
					}
				} else {
					PL_LOG_ERROR("Package: '{}' has language module dependency: '{}', but it was not found.", package.name, lang);
					_conflictedPackages.emplace_back(&package);
					continue;
				}
			}

			for (const auto& dependency : pluginDescriptor->dependencies) {
				if (dependency.optional || !IsSupportsPlatform(dependency.supportedPlatforms))
					continue;

				auto itl = _localPackages.find(dependency.name);
				if (itl != _localPackages.end()) {
					auto localPackage = std::get<LocalPackage>(*itl);
					if (dependency.requestedVersion.has_value() && *dependency.requestedVersion != localPackage.version)  {
						PL_LOG_ERROR("Package: '{}' has dependency: '{}' which required (v{}), but (v{}) installed. Conflict cannot be resolved automatically.", package.name, dependency.name, *dependency.requestedVersion, localPackage.version);
					}
					continue;
				}

				auto itr = _remotePackages.find(dependency.name);
				if (itr != _remotePackages.end()) {
					auto remotePackage = std::get<RemotePackage>(*itr);
					if (dependency.requestedVersion.has_value() && !remotePackage.Version(*dependency.requestedVersion)) {
						PL_LOG_ERROR("Package: '{}' has dependency: '{}' which required (v{}), but version was not found. Problem cannot be resolved automatically.", package.name, dependency.name, *dependency.requestedVersion);
						_conflictedPackages.emplace_back(&package);
						continue;
					}

					auto it = _missedPackages.find(dependency.name);
					if (it == _missedPackages.end()) {
						_missedPackages.emplace(dependency.name, std::pair{ &remotePackage, dependency.requestedVersion });
					} else {
						auto& existingDependency = std::get<Dependency>(*it);

						auto& existingVersion = existingDependency.second;
						if (dependency.requestedVersion.has_value()) {
							if (existingVersion.has_value()) {
								if (*existingVersion != *dependency.requestedVersion) {
									PL_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' dependency, over older version (v{}).", std::max(*existingVersion, *dependency.requestedVersion), dependency.name, std::min(*existingVersion, *dependency.requestedVersion));

									if (*existingVersion < *dependency.requestedVersion) {
										existingVersion = dependency.requestedVersion;
									}
								} else {
									PL_LOG_WARNING("The same version (v{}) of dependency '{}' required by '{}' at '{}' - second one will be ignored.", *existingVersion, dependency.name, package.name, package.path.string());
								}
							} else {
								existingVersion = dependency.requestedVersion;
							}
						}
					}
				} else {
					PL_LOG_ERROR("Package: '{}' has dependency: '{}' which could not be found.", package.name, dependency.name);
					_conflictedPackages.emplace_back(&package);
				}
			}
		}
	}

	for (const auto& [_, dependency] : _missedPackages) {
		const auto& [package, version] = dependency;
		PL_LOG_INFO("Required to install: '{}' [{}] (v{})", package->name, package->type, version.has_value() ? std::to_string(*version) : "[latest]");
	}

	for (const auto& package : _conflictedPackages) {
		PL_LOG_WARNING("Unable to install: '{}' [{}] (v{}) due to unresolved conflicts", package->name, package->type, package->version);
	}
}

void PackageManager::InstallMissedPackages() {
	Request([&]{
		std::string missed;
		bool first = true;
		for (const auto& [name, dependency] : _missedPackages) {
			const auto& [package, version] = dependency;
			InstallPackage(*package, version);
			if (first) {
				std::format_to(std::back_inserter(missed), "'{}", name);
				first = false;
			} else {
				std::format_to(std::back_inserter(missed), "', '{}", name);
			}
		}
		if (!first) {
			std::format_to(std::back_inserter(missed), "'");
			PL_LOG_INFO("Trying install {} missing package(s) to solve dependency issues", missed);
		}
	}, __func__);
}

void PackageManager::UninstallConflictedPackages() {
	Request([&]{
		std::string conflicted;
		bool first = true;
		for (const auto& package : _conflictedPackages) {
			std::string name(package->name);
			UninstallPackage(*package);
			if (first) {
				std::format_to(std::back_inserter(conflicted), "'{}", name);
				first = false;
			} else {
				std::format_to(std::back_inserter(conflicted), "', '{}", name);
			}
		}
		if (!first) {
			std::format_to(std::back_inserter(conflicted), "'");
			PL_LOG_INFO("Trying uninstall {} conflicted package(s) to solve dependency issues", conflicted);
		}
	}, __func__);
}

void PackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) {
	auto debugStart = DateTime::Now();

	std::unordered_map<std::string, RemotePackage> packages;
	packages.reserve(_localPackages.size());

	for (const auto& [name, package] : _localPackages) {
		packages.emplace(name, package);
	}

	if (packages.empty()) {
		PL_LOG_WARNING("Packages was not found!");
		return;
	}

	PackageManifest manifest{ std::move(packages) };
	std::string buffer;
	const auto ec = glz::write_json(manifest, buffer);
	if (ec) {
		PL_LOG_ERROR("Snapshot packages: JSON writing error: {}", glz::format_error(ec));
		return;
	}
	FileSystem::WriteText(manifestFilePath, prettify ? glz::prettify_json(buffer) : buffer);

	PL_LOG_DEBUG("Snapshot '{}' created in {}ms", manifestFilePath.string(), (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

void PackageManager::InstallPackage(std::string_view packageName, std::optional<int32_t> requiredVersion) {
	if (packageName.empty())
		return;

	Request([&] {
		auto it = _remotePackages.find(packageName);
		if (it != _remotePackages.end()) {
			InstallPackage(std::get<RemotePackage>(*it), requiredVersion);
		} else {
			PL_LOG_ERROR("Package: {} not found", packageName);
		}
	}, __func__);
}

void PackageManager::InstallPackages(std::span<const std::string> packageNames) {
	std::unordered_set<std::string> unique;
	unique.reserve(packageNames.size());
	Request([&] {
		std::string error;
		bool first = true;
		for (const auto& packageName: packageNames) {
			if (packageName.empty() || unique.contains(packageName))
				continue;
			auto it = _remotePackages.find(packageName);
			if (it != _remotePackages.end()) {
				InstallPackage(std::get<RemotePackage>(*it));
			} else {
				if (first) {
					std::format_to(std::back_inserter(error), "'{}", packageName);
					first = false;
				} else {
					std::format_to(std::back_inserter(error), "', '{}", packageName);
				}
			}
			unique.insert(packageName);
		}
		if (!first) {
			std::format_to(std::back_inserter(error), "'");
			PL_LOG_ERROR("Not found {} packages(s)", error);
		}
	}, __func__);
}

void PackageManager::InstallAllPackages(const fs::path& manifestFilePath, bool reinstall) {
	if (manifestFilePath.extension().string() != PackageManifest::kFileExtension) {
		PL_LOG_ERROR("Package manifest: '{}' should be in *{} format", manifestFilePath.string(), PackageManifest::kFileExtension);
		return;
	}

	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	auto path = plugify->GetConfig().baseDir / manifestFilePath;

	PL_LOG_INFO("Read package manifest from '{}'", path.string());

	auto json = FileSystem::ReadText(path);
	auto manifest = glz::read_json<PackageManifest>(json);
	if (!manifest.has_value()) {
		PL_LOG_ERROR("Package manifest: '{}' has JSON parsing error: {}", path.string(), glz::format_error(manifest.error(), json));
		return;
	}

	if (!reinstall) {
		for (const auto& [name, _] : _localPackages) {
			manifest->content.erase(name);
		}
	}

	if (manifest->content.empty()) {
		PL_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
		return;
	}

	Request([&] {
		for (auto& [name, package]: manifest->content) {
			if (name.empty() || package.name != name) {
				PL_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", path.string(), name, package.name);
				continue;
			}
			RemoveUnsupported(package);
			if (package.versions.empty()) {
				PL_LOG_ERROR("Package manifest: '{}' has empty version list at '{}'", path.string(), name);
				continue;
			}
			InstallPackage(package);
		}
	}, __func__);
}

void PackageManager::InstallAllPackages(const std::string& manifestUrl, bool reinstall) {
	if (!String::IsValidURL(manifestUrl)) {
		PL_LOG_WARNING("Tried to install packages from manifest which is not have valid url: \"{}\", aborting", manifestUrl);
		return;
	}

	PL_LOG_INFO("Read package manifest from '{}'", manifestUrl);

	const char* func = __func__;

	_httpDownloader->CreateRequest(manifestUrl, [&](int32_t statusCode, std::string_view, HTTPDownloader::Request::Data data) {
		if (statusCode == HTTPDownloader::HTTP_STATUS_OK) {
			/*if (contentType != "text/plain" || contentType != "application/json" || contentType != "text/json" || contentType != "text/javascript") {
				PL_LOG_ERROR("Package manifest: '{}' should be in text format to be read correctly", manifestUrl);
				return;
			}*/

			std::string buffer(data.begin(), data.end());
			auto manifest = glz::read_json<PackageManifest>(buffer);
			if (!manifest.has_value()) {
				PL_LOG_ERROR("Packages manifest from '{}' has JSON parsing error: {}", manifestUrl, glz::format_error(manifest.error(), buffer));
				return;
			}

			if (!reinstall) {
				for (const auto& [name, _] : _localPackages) {
					manifest->content.erase(name);
				};
			}

			if (manifest->content.empty()) {
				PL_LOG_WARNING("No packages to install was found! If you need to reinstall all installed packages, use the reinstall flag!");
				return;
			}

			Request([&] {
				for (auto& [name, package] : manifest->content) {
					if (name.empty() || package.name != name) {
						PL_LOG_ERROR("Package manifest: '{}' has different name in key and object: {} <-> {}", manifestUrl, name, package.name);
						continue;
					}
					RemoveUnsupported(package);
					if (package.versions.empty()) {
						PL_LOG_ERROR("Package manifest: '{}' has empty version list at '{}'", manifestUrl, name);
						continue;
					}
					InstallPackage(package);
				}
			}, func);
		}
	});

	_httpDownloader->WaitForAllRequests();
}

bool PackageManager::InstallPackage(const RemotePackage& package, std::optional<int32_t> requiredVersion) {
	auto it = _localPackages.find(package.name);
	if (it != _localPackages.end()) {
		PL_LOG_WARNING("Package: '{}' (v{}) already installed", package.name, std::get<LocalPackage>(*it).version);
		return false;
	}

	PackageOpt newVersion;
	if (requiredVersion.has_value()) {
		newVersion = package.Version(*requiredVersion);
		if (newVersion) {
			if (!IsSupportsPlatform(newVersion->platforms))
				return false;
		} else {
			PL_LOG_WARNING("Package: '{}' (v{}) has not been found", package.name, *requiredVersion);
			return false;
		}
	} else {
		newVersion = package.LatestVersion();
		if (newVersion) {
			if (!IsSupportsPlatform(newVersion->platforms))
				return false;
		} else {
			PL_LOG_WARNING("Package: '{}' (v[latest]]) has not been found", package.name);
			return false;
		}
	}

	return DownloadPackage(package, *newVersion);
}

void PackageManager::UpdatePackage(std::string_view packageName, std::optional<int32_t> requiredVersion) {
	if (packageName.empty())
		return;

	Request([&] {
		auto it = _localPackages.find(packageName);
		if (it != _localPackages.end()) {
			UpdatePackage(std::get<LocalPackage>(*it), requiredVersion);
		} else {
			PL_LOG_ERROR("Package: {} not found", packageName);
		}
	}, __func__);
}

void PackageManager::UpdatePackages(std::span<const std::string> packageNames) {
	std::unordered_set<std::string> unique;
	unique.reserve(packageNames.size());
	Request([&] {
		std::string error;
		bool first = true;
		for (const auto& packageName: packageNames) {
			if (packageName.empty() || unique.contains(packageName))
				continue;
			auto it = _localPackages.find(packageName);
			if (it != _localPackages.end()) {
				UpdatePackage(std::get<LocalPackage>(*it));
			} else {
				if (first) {
					std::format_to(std::back_inserter(error), "'{}", packageName);
					first = false;
				} else {
					std::format_to(std::back_inserter(error), "', '{}", packageName);
				}
			}
			unique.insert(packageName);
		}
		if (!first) {
			std::format_to(std::back_inserter(error), "'");
			PL_LOG_ERROR("Not found {} packages(s)", error);
		}
	}, __func__);
}

void PackageManager::UpdateAllPackages() {
	Request([&] {
		for (const auto& [_, package] : _localPackages) {
			UpdatePackage(package);
		}
	}, __func__);
}

bool PackageManager::UpdatePackage(const LocalPackage& package, std::optional<int32_t> requiredVersion) {
	auto it = _remotePackages.find(package.name);
	if (it == _remotePackages.end()) {
		PL_LOG_WARNING("Package: '{}' has not been found", package.name);
		return false;
	}

	const auto& newPackage =  std::get<RemotePackage>(*it);
	PackageOpt newVersion;
	if (requiredVersion.has_value()) {
		newVersion = newPackage.Version(*requiredVersion);
		if (newVersion) {
			if (!IsSupportsPlatform(newVersion->platforms))
				return false;

			PL_LOG_INFO("Package '{}' (v{}) will be {}, to different version (v{})", package.name, package.version, newVersion->version > package.version ? "upgraded" : newVersion->version == package.version ? "reinstalled" : "downgraded", newVersion->version);
		} else {
			PL_LOG_WARNING("Package: '{}' (v{}) has not been found", package.name, *requiredVersion);
			return false;
		}
	} else {
		newVersion = newPackage.LatestVersion();
		if (newVersion) {
			if (!IsSupportsPlatform(newVersion->platforms))
				return false;

			if (newVersion->version > package.version) {
				PL_LOG_INFO("Update available, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(package.version, newVersion->version), newPackage.name, std::min(package.version, newVersion->version));
			} else {
				PL_LOG_WARNING("Package: '{}' has no update available", package.name);
				return false;
			}
		} else {
			PL_LOG_WARNING("Package: '{}' (v[latest]) has not been found", package.name);
			return false;
		}
	}

	return DownloadPackage(package, *newVersion);
}

void PackageManager::UninstallPackage(std::string_view packageName) {
	if (packageName.empty())
		return;

	Request([&] {
		auto it = _localPackages.find(packageName);
		if (it != _localPackages.end()) {
			UninstallPackage(std::get<LocalPackage>(*it));
		} else {
			PL_LOG_ERROR("Package: {} not found", packageName);
		}
	}, __func__);
}

void PackageManager::UninstallPackages(std::span<const std::string> packageNames) {
	std::unordered_set<std::string> unique;
	unique.reserve(packageNames.size());
	Request([&] {
		std::string error;
		bool first = true;
		for (const auto& packageName: packageNames) {
			if (packageName.empty() || unique.contains(packageName))
				continue;
			auto it = _localPackages.find(packageName);
			if (it != _localPackages.end()) {
				UninstallPackage(std::get<LocalPackage>(*it));
			} else {
				if (first) {
					std::format_to(std::back_inserter(error), "'{}", packageName);
					first = false;
				} else {
					std::format_to(std::back_inserter(error), "', '{}", packageName);
				}
			}
			unique.insert(packageName);
		}
		if (!first) {
			std::format_to(std::back_inserter(error), "'");
			PL_LOG_ERROR("Not found {} packages(s)", error);
		}
	}, __func__);
}

void PackageManager::UninstallAllPackages() {
	Request([&] {
		for (const auto& [_, package] : _localPackages) {
			UninstallPackage(package, false);
		}
		_localPackages.clear();
	}, __func__);
}

bool PackageManager::UninstallPackage(const LocalPackage& package, bool remove) {
	PL_ASSERT(package.path.has_parent_path(), "Package path doesn't contain parent path");
	auto packagePath = package.path.parent_path();
	std::error_code ec = FileSystem::RemoveFolder(packagePath);
	if (!ec) {
		if (remove)
			_localPackages.erase(package.name);
		PL_LOG_INFO("Package: '{}' (v{}) was removed from: '{}'", package.name, package.version, packagePath.string());
		return true;
	}
	return false;
}

void PackageManager::Request(const std::function<void()>& action, std::string_view function) {
	auto debugStart = DateTime::Now();

	action();

	_httpDownloader->WaitForAllRequests();

	LoadAllPackages();

	PL_LOG_DEBUG("{} processed in {}ms", function, (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

bool PackageManager::DownloadPackage(const Package& package, const PackageVersion& version) const {
	if (!String::IsValidURL(version.download)) {
		PL_LOG_WARNING("Tried to download a package: '{}' that is not have valid url: \"{}\", aborting", package.name, version.download.empty() ? "<empty>" : version.download);
		return false;
	}

	PL_LOG_VERBOSE("Start downloading: '{}'", package.name);

	auto plugify = _plugify.lock();
	PL_ASSERT(plugify);

	PL_LOG_INFO("Downloading: '{}'", version.download);

	_httpDownloader->CreateRequest(version.download, [&name = package.name, plugin = (package.type == "plugin"), &baseDir = plugify->GetConfig().baseDir, &checksum = version.checksum] // should be safe to pass ref
		(int32_t statusCode, std::string_view, HTTPDownloader::Request::Data data) {
		if (statusCode == HTTPDownloader::HTTP_STATUS_OK) {
			PL_LOG_VERBOSE("Done downloading: '{}'", name);

			/*if (contentType != "application/zip") {
				PL_LOG_ERROR("Package: '{}' should be in *.zip format to be extracted correctly", name);
				return;
			}*/

			if (!IsPackageLegit(checksum, data)) {
				PL_LOG_WARNING("Archive hash '{}' does not match expected checksum, aborting", name);
				return;
			}

			const auto& [folder, extension] = packageTypes[plugin];

			fs::path finalPath = baseDir / folder;
			fs::path finalLocation = finalPath / std::format("{}-{}", name, DateTime::Get("%Y_%m_%d_%H_%M_%S"));

			std::error_code ec;
			if (!fs::exists(finalLocation, ec) || !fs::is_directory(finalLocation, ec)) {
				if (!fs::create_directories(finalLocation, ec)) {
					PL_LOG_ERROR("Error creating output directory '{}'", finalLocation.string());
				}
			}

			auto error = ExtractPackage(data, finalLocation, extension);
			if (error.empty()) {
				PL_LOG_VERBOSE("Done extracting: '{}'", name);
				auto destinationPath = finalPath / name;
				ec = FileSystem::MoveFolder(finalLocation, destinationPath);
				if (ec) {
					PL_LOG_ERROR("Package: '{}' could be renamed from '{}' to '{}' - {}", name, finalLocation.string(), destinationPath.string(), ec.message());
				} else {
					PL_LOG_VERBOSE("Package: '{}' was renamed successfully from '{}' to '{}'", name, finalLocation.string(), destinationPath.string());
				}

			} else {
				PL_LOG_ERROR("Failed extracting: '{}' - {}", name, error);
			}
		} else {
			PL_LOG_ERROR("Failed downloading: '{}' - Code: {}", name, statusCode);
		}
	});

	return true;
}

std::string PackageManager::ExtractPackage(std::span<const uint8_t> packageData, const fs::path& extractPath, std::string_view descriptorExt) {
	PL_LOG_VERBOSE("Start extracting: '{}' ....", extractPath.string());

	auto zipClose = [](mz_zip_archive* zipArchive){ mz_zip_reader_end(zipArchive); delete zipArchive; };
	std::unique_ptr<mz_zip_archive, decltype(zipClose)> zipArchive(new mz_zip_archive, zipClose);
	std::memset(zipArchive.get(), 0, sizeof(mz_zip_archive));

	mz_zip_reader_init_mem(zipArchive.get(), packageData.data(), packageData.size(), 0);

	//state.total = zipArchive->m_archive_size;
	//state.progress = 0;

	size_t numFiles = mz_zip_reader_get_num_files(zipArchive.get());
	std::vector<mz_zip_archive_file_stat> fileStats(numFiles);

	bool foundDescriptor = false;

	for (uint32_t i = 0; i < numFiles; ++i) {
		mz_zip_archive_file_stat& fileStat = fileStats[i];

		if (!mz_zip_reader_file_stat(zipArchive.get(), i, &fileStat)) {
			return std::format("Error getting file stat: {}", i);
		}

		fs::path filename(fileStat.m_filename);
		if (filename.extension().string() == descriptorExt) {
			foundDescriptor = true;
		}
	}

	if (!foundDescriptor) {
		return std::format("Package descriptor *{} missing", descriptorExt);
	}

	for (uint32_t i = 0; i < numFiles; ++i) {
		mz_zip_archive_file_stat& fileStat = fileStats[i];

		std::vector<char> fileData(static_cast<size_t>(fileStat.m_uncomp_size));

		if (!mz_zip_reader_extract_to_mem(zipArchive.get(), i, fileData.data(), fileData.size(), 0)) {
			return std::format("Failed extracting file: '{}'", fileStat.m_filename);
		}

		std::error_code ec;
		fs::path finalPath = extractPath / fileStat.m_filename;

		if (fileStat.m_is_directory) {
			fs::create_directories(finalPath, ec);
		} else {
			fs::create_directories(finalPath.parent_path(), ec);

			std::ofstream outputFile(finalPath, std::ios::binary);
			if (outputFile.is_open()) {
				outputFile.write(fileData.data(), static_cast<std::streamsize>(fileData.size()));
			} else {
				return std::format("Failed creating destination file: '{}'", fileStat.m_filename);
			}

			//state.progress += fileStat.m_comp_size;
			//state.ratio = std::roundf(static_cast<float>(_packageState.progress) / static_cast<float>(_packageState.total) * 100.0f);
		}
	}

	return {};
}

bool PackageManager::IsPackageLegit(std::string_view checksum, std::span<const uint8_t> packageData) {
	if (checksum.empty())
		return true;

	Sha256 sha;
	sha.update(packageData);
	std::string hash(Sha256::ToString(sha.digest()));

	PL_LOG_VERBOSE("Expected checksum: {}", checksum);
	PL_LOG_VERBOSE("Computed checksum: {}", hash);

	return checksum == hash;
}

#else

void PackageManager::InstallPackage(std::string_view /*packageName*/, std::optional<int32_t> /*requiredVersion*/) {}
void PackageManager::InstallPackages(std::span<const std::string> /*packageNames*/) {}
void PackageManager::InstallAllPackages(const fs::path& /*manifestFilePath*/, bool /*reinstall*/) {}
void PackageManager::InstallAllPackages(const std::string& /*manifestUrl*/, bool /*reinstall*/) {}

void PackageManager::UpdatePackage(std::string_view /*packageName*/, std::optional<int32_t> /*requiredVersion*/) {}
void PackageManager::UpdatePackages(std::span<const std::string> /*packageNames*/) {}
void PackageManager::UpdateAllPackages() {}

void PackageManager::UninstallPackage(std::string_view /*packageName*/) {}
void PackageManager::UninstallPackages(std::span<const std::string> /*packageNames*/) {}
void PackageManager::UninstallAllPackages() {}

void PackageManager::SnapshotPackages(const fs::path& /*manifestFilePath*/, bool /*reinstall*/) {}

void PackageManager::InstallMissedPackages() {}
void PackageManager::UninstallConflictedPackages() {}

#endif // PLUGIFY_DOWNLOADER

bool PackageManager::HasMissedPackages() const {
	return !_missedPackages.empty();
}

bool PackageManager::HasConflictedPackages() const {
	return !_conflictedPackages.empty();
}

LocalPackageOpt PackageManager::FindLocalPackage(std::string_view packageName) const {
	auto it = _localPackages.find(packageName);
	if (it != _localPackages.end())
		return std::get<LocalPackage>(*it);
	return {};
}

RemotePackageOpt PackageManager::FindRemotePackage(std::string_view packageName) const {
	auto it = _remotePackages.find(packageName);
	if (it != _remotePackages.end())
		return std::get<RemotePackage>(*it);
	return {};
}

std::vector<LocalPackage> PackageManager::GetLocalPackages() const {
	std::vector<LocalPackage> localPackages;
	localPackages.reserve(_localPackages.size());
	for (const auto& [_, package] : _localPackages)  {
		localPackages.emplace_back(package);
	}
	return localPackages;
}

std::vector<RemotePackage> PackageManager::GetRemotePackages() const {
	std::vector<RemotePackage> remotePackages;
	remotePackages.reserve(remotePackages.size());
	for (const auto& [_, package] : _remotePackages)  {
		remotePackages.emplace_back(package);
	}
	return remotePackages;
}
