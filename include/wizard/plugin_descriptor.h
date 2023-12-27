#pragma once

#include <wizard/language_module_descriptor.h>
#include <wizard/plugin_reference_descriptor.h>
#include <wizard/method.h>
#include <filesystem>
#include <vector>

namespace wizard {
    struct PluginDescriptor {
        std::int32_t fileVersion{ 0 };
        std::int32_t version{ 0 };
        std::string versionName;
        std::string friendlyName;
        std::string description;
        std::string createdBy;
        std::string createdByURL;
        std::string docsURL;
        std::string downloadURL;
        std::string supportURL;
        std::filesystem::path assemblyPath;
        std::vector<std::string> supportedPlatforms;
        LanguageModuleInfo languageModule;
        std::vector<PluginReferenceDescriptor> dependencies;
        std::vector<Method> exportedMethods;

#if WIZARD_BUILD_MAIN_LIB
        PluginDescriptor() = default;

        bool IsSupportsPlatform(const std::string& platform) const;

	    bool Load(const fs::path& filePath);
        bool Read(const utils::json::Value& object);

        static inline const char* const kFileExtension = ".wplugin";
#endif
    };
}