#include "package_manager.h"
#include "module.h"
#include "plugin.h"
#include "wizard/package_manager.h"


#include <wizard/wizard.h>
#include <utils/file_system.h>
#include <utils/json.h>

using namespace wizard;

PackageManager::PackageManager(std::weak_ptr<IWizard> wizard) : IPackageManager(*this), WizardContext(std::move(wizard)) {
	auto debugStart = DateTime::Now();
	//Initialize();
	WZ_LOG_DEBUG("PackageManager loaded in {}ms", (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

PackageManager::~PackageManager() = default;

void PackageManager::LoadPackages() {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	std::vector<fs::path> manifestsFilePaths = FileSystem::GetFiles(wizard->GetConfig().baseDir, true, { PackageManifest::kFileExtension });

	for (const auto& path : manifestsFilePaths) {
		WZ_LOG_INFO("Read package manifest from '{}'", path.string());

		auto json = FileSystem::ReadText(path);
		auto manifest = glz::read_json<PackageManifest>(json);
		if (!manifest.has_value()) {
			WZ_LOG_ERROR("Package manifest: '{}' has JSON parsing error: {}", path.string(), glz::format_error(manifest.error(), json));
			continue;
		}

		std::vector<const std::string*> packages;

		for (const auto& [name, package] : manifest->content) {
			auto it = _allPackages.find(name);
			if (it == _allPackages.end()) {
				packages.push_back(&name);
			} else {
				const auto& existingPackage = std::get<Package>(*it);

				auto& existingVersion = existingPackage.version;
				if (existingVersion != package.version) {
					WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package.version), name, std::min(existingVersion, package.version));

					if (existingVersion < package.version) {
						packages.push_back(&name);
					}
				} else {
					WZ_LOG_WARNING("The same version (v{}) of package '{}' exists at '{}' and '{}' - second location will be ignored.", existingVersion, name, existingPackage.url, path.string());
				}
			}
		}

		for (const std::string* name : packages) {
			_allPackages[*name] = std::move(manifest->content[*name]);
		}
	}
}

void PackageManager::SnapshotPackages(const fs::path& filepath, bool prettify) {
	auto wizard = _wizard.lock();
	if (!wizard)
		return;

	auto debugStart = DateTime::Now();

	std::unordered_map<std::string, Package> packages;

	std::vector<fs::path> filePaths = FileSystem::GetFiles(wizard->GetConfig().baseDir, true);

	for (const auto& path : filePaths) {
		std::string extension{ path.extension().string() };
		if (extension != Module::kFileExtension && extension != Plugin::kFileExtension)
			continue;

		std::string name{ path.filename().replace_extension().string() };

		auto package = ReadDescriptor(path, name, extension == Module::kFileExtension);
		if (!package.has_value())
			continue;

		auto it = packages.find(name);
		if (it == packages.end()) {
			packages.emplace(std::move(name), std::move(*package));
		} else {
			const auto& existingPackage = std::get<Package>(*it);

			auto& existingVersion = existingPackage.version;
			if (existingVersion != package->version) {
				WZ_LOG_WARNING("By default, prioritizing newer version (v{}) of '{}' package, over older version (v{}).", std::max(existingVersion, package->version), name, std::min(existingVersion, package->version));

				if (existingVersion < package->version) {
					packages[std::move(name)] = std::move(*package);
				}
			} else {
				WZ_LOG_WARNING("The same version (v{}) of package '{}' exists at '{}' - second location will be ignored.", existingVersion, name, path.string());
			}
		}
	}

	if (packages.empty()) {
		WZ_LOG_WARNING("Packages was not found!");
		return;
	}

	PackageManifest manifest{ std::move(packages) };
	std::string buffer;
	glz::write_json(manifest, buffer);
	if (prettify) glz::prettify(buffer);
	FileSystem::WriteText(filepath, buffer);

	WZ_LOG_DEBUG("Snapshot '{}' created in {}ms", filepath.string(), (DateTime::Now() - debugStart).AsMilliseconds<float>());
}

std::optional<Package> PackageManager::ReadDescriptor(const fs::path& path, const std::string& name, bool module) {
	if (module) {
		auto json = FileSystem::ReadText(path);
		auto descriptor = glz::read_json<LanguageModuleDescriptor>(json);
		if (!descriptor.has_value()) {
			WZ_LOG_ERROR("Module descriptor: {} has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
			return {};
		}
		return std::make_optional<Package>(name, descriptor->downloadURL, descriptor->version, false, true);
	} else {
		auto json = FileSystem::ReadText(path);
		auto descriptor = glz::read_json<PluginDescriptor>(json);
		if (!descriptor.has_value()) {
			WZ_LOG_ERROR("Plugin descriptor: '{}' has JSON parsing error: {}", name, glz::format_error(descriptor.error(), json));
			return {};
		}
		return std::make_optional<Package>(name, descriptor->downloadURL, descriptor->version, false, false);
	}
}
