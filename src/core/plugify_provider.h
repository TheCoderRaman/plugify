#pragma once

#include "plugify_context.h"
#include <plugify/plugify_provider.h>
#include <plugify/plugify.h>

namespace plugify {
	class PlugifyProvider final : public IPlugifyProvider, public PlugifyContext {
	public:
		explicit PlugifyProvider(std::weak_ptr<IPlugify> plugify);
		~PlugifyProvider();

		void Log(std::string_view msg, Severity severity);

		const fs::path& GetBaseDir() noexcept;

		bool IsPreferOwnSymbols() noexcept;
		
		bool IsPluginLoaded(std::string_view name, std::optional<int32_t> requiredVersion = {}, bool minimum = false) noexcept;
		
		bool IsModuleLoaded(std::string_view name, std::optional<int32_t> requiredVersion = {}, bool minimum = false) noexcept;
	};
}
