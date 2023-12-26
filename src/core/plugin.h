#pragma once

#include <wizard/plugin.h>
#include <wizard/plugin_descriptor.h>

namespace wizard {
    enum class PluginState : uint8_t {
        NotLoaded,
        Error,
        Loaded,
        Running,
        Terminating,
        Unloaded
    };

    class Module;
    class Plugin final : public IPlugin {
    public:
        Plugin(uint64_t id, std::string name, fs::path filePath, PluginDescriptor descriptor);
        ~Plugin() = default;

        /* IPlugin interface */
        uint64_t GetId() const override {
            return _id;
        }

        const std::string& GetName() const override {
            return _name;
        }

        const std::string& GetFriendlyName() const override {
            return GetDescriptor().friendlyName.empty() ? GetName() : GetDescriptor().friendlyName;
        }

        const fs::path& GetFilePath() const override {
            return _filePath;
        }

        // TODO: Implement
        fs::path GetBaseDir() const override {
            return "";
        }
        fs::path GetContentDir() const override {
            return "";
        }
        fs::path GetMountedAssetPath() const override {
            return "";
        }

        const PluginDescriptor& GetDescriptor() const override {
            return _descriptor;
        }

        std::shared_ptr<Module> GetModule() const {
            return _module;
        }

        void SetModule(std::shared_ptr<Module> module) {
            _module = std::move(module);
        }

        PluginState GetState() const {
            return _state;
        }

        void SetError(std::string error);

        void SetLoaded() {
            _state = PluginState::Loaded;
        }

        void SetRunning() {
            _state = PluginState::Running;
        }

        void SetTerminating() {
            _state = PluginState::Terminating;
        }

        void SetUnloaded() {
            _state = PluginState::Unloaded;
        }

    private:
        uint64_t _id{ std::numeric_limits<uint64_t>::max() };
        std::string _name;
        fs::path _filePath;
        std::string _error;
        std::shared_ptr<Module> _module;
        PluginDescriptor _descriptor;
        PluginState _state{ PluginState::NotLoaded };
    };
}
