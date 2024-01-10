#include <wizard/plugin_manager.h>
#include <core/plugin_manager.h>

using namespace wizard;

IPluginManager::IPluginManager(PluginManager& impl) : _impl{impl} {
}

ModuleRef IPluginManager::FindModule(const std::string& moduleName) {
    return _impl.FindModule(moduleName);
}

ModuleRef IPluginManager::FindModule(std::string_view moduleName) {
    return _impl.FindModule(moduleName);
}

ModuleRef IPluginManager::FindModuleFromId(std::uint64_t moduleId) {
    return _impl.FindModuleFromId(moduleId);
}

ModuleRef IPluginManager::FindModuleFromLang(const std::string& moduleLang) {
    return _impl.FindModuleFromLang(moduleLang);
}

ModuleRef IPluginManager::FindModuleFromPath(const fs::path& moduleFilePath) {
    return _impl.FindModuleFromPath(moduleFilePath);
}

ModuleRef IPluginManager::FindModuleFromDescriptor(const PluginReferenceDescriptor& moduleDescriptor) {
    return _impl.FindModuleFromDescriptor(moduleDescriptor);
}

std::vector<std::reference_wrapper<const IModule>> IPluginManager::GetModules() {
    return _impl.GetModules();
}

PluginRef IPluginManager::FindPlugin(const std::string& pluginName) {
    return _impl.FindPlugin(pluginName);
}

PluginRef IPluginManager::FindPlugin(std::string_view pluginName) {
    return _impl.FindPlugin(pluginName);
}

PluginRef IPluginManager::FindPluginFromId(uint64_t pluginId) {
    return _impl.FindPluginFromId(pluginId);
}

PluginRef IPluginManager::FindPluginFromPath(const fs::path& pluginFilePath) {
    return _impl.FindPluginFromPath(pluginFilePath);
}

PluginRef IPluginManager::FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) {
    return _impl.FindPluginFromDescriptor(pluginDescriptor);
}

std::vector<std::reference_wrapper<const IPlugin>> IPluginManager::GetPlugins() {
    return _impl.GetPlugins();
}

bool IPluginManager::GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
    return _impl.GetPluginDependencies(pluginName, pluginDependencies);
}

bool IPluginManager::GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
    return _impl.GetPluginDependencies_FromFilePath(pluginFilePath, pluginDependencies);
}

bool IPluginManager::GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) {
    return _impl.GetPluginDependencies_FromDescriptor(pluginDescriptor, pluginDependencies);
}
