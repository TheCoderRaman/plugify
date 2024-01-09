#include <wizard/package_manager.h>
#include <core/package_manager.h>

using namespace wizard;

IPackageManager::IPackageManager(PackageManager& impl) : _impl{impl} {
}

void IPackageManager::LoadLocalPackages() {
	return _impl.LoadLocalPackages();
}

void IPackageManager::LoadRemotePackages() {
	return _impl.LoadRemotePackages();
}

void IPackageManager::InstallPackage(const std::string& packageName, std::optional<int32_t> requiredVersion) {
	return _impl.InstallPackage(packageName, requiredVersion);
}

void IPackageManager::InstallPackages(std::span<const std::string> packageNames) {
	return _impl.InstallPackages(packageNames);
}

void IPackageManager::InstallAllPackages(const fs::path& manifestFilePath, bool reinstall) {
	return _impl.InstallAllPackages(manifestFilePath, reinstall);
}

void IPackageManager::InstallAllPackages(const std::string& manifestUrl, bool reinstall) {
	return _impl.InstallAllPackages(manifestUrl, reinstall);
}

void IPackageManager::UpdatePackage(const std::string& packageName, std::optional<int32_t> requiredVersion) {
	return _impl.UpdatePackage(packageName, requiredVersion);
}

void IPackageManager::UpdatePackages(std::span<const std::string> packageNames) {
	return _impl.UpdatePackages(packageNames);
}

void IPackageManager::UpdateAllPackages() {
	return _impl.UpdateAllPackages();
}

void IPackageManager::UninstallPackage(const std::string& packageName) {
	return _impl.UninstallPackage(packageName);
}

void IPackageManager::UninstallPackages(std::span<const std::string> packageNames) {
	return _impl.UninstallPackages(packageNames);
}

void IPackageManager::UninstallAllPackages() {
	return _impl.UninstallAllPackages();
}

void IPackageManager::SnapshotPackages(const fs::path& manifestFilePath, bool prettify) const {
	return _impl.SnapshotPackages(manifestFilePath, prettify);
}

LocalPackageRef IPackageManager::FindLocalPackage(const std::string& packageName) const {
	return _impl.FindLocalPackage(packageName);
}

RemotePackageRef IPackageManager::FindRemotePackage(const std::string& packageName) const {
	return _impl.FindRemotePackage(packageName);
}
