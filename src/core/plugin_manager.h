#pragma once

#include <wizard/plugin_manager.h>
#include <wizard/plugin.h>
#include <wizard/language_module.h>

namespace wizard {
    class Plugin;
    class Module;

    class PluginManager final : public IPluginManager {
    public:
        PluginManager();
        ~PluginManager();

        /** IPluginManager interface */
        std::shared_ptr<IPlugin> FindPlugin(const std::string& pluginName) override;
        std::shared_ptr<IPlugin> FindPlugin(std::string_view pluginName) override;
        std::shared_ptr<IPlugin> FindPluginFromId(uint64_t pluginId) override;
        std::shared_ptr<IPlugin> FindPluginFromPath(const fs::path& pluginFilePath) override;
        std::shared_ptr<IPlugin> FindPluginFromDescriptor(const PluginReferenceDescriptor& pluginDescriptor) override;
        std::vector<std::shared_ptr<IPlugin>> GetPlugins() override;
        bool GetPluginDependencies(const std::string& pluginName, std::vector<PluginReferenceDescriptor>& pluginDependencies) override;
        bool GetPluginDependencies_FromFilePath(const fs::path& pluginFilePath, std::vector<PluginReferenceDescriptor>& pluginDependencies) override;
        bool GetPluginDependencies_FromDescriptor(const PluginReferenceDescriptor& pluginDescriptor, std::vector<PluginReferenceDescriptor>& pluginDependencies) override;

    private:
        using PluginList = std::vector<std::shared_ptr<Plugin>>;
        using ModuleMap = std::unordered_map<std::string, std::shared_ptr<Module>>;
        using VisitedPluginMap = std::unordered_map<std::string, std::pair<bool, bool>>;

        void DiscoverAllPlugins();
        void DiscoverAllModules();
        void ReadAllPluginsDescriptors();

        void LoadRequiredLanguageModules();
        void LoadPlugins();

        static void SortPluginsByDependencies(const std::string& pluginName, PluginList& sourceList, PluginList& targetList);
        static bool HasCyclicDependencies(PluginList& plugins);
        static bool IsCyclic(const std::shared_ptr<Plugin>& plugin, PluginList& plugins, VisitedPluginMap& visitedPlugins);

    private:
        PluginList _allPlugins;
        ModuleMap _allModules;
    };
}