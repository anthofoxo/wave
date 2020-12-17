#pragma once

#include <memory>

#include <spdlog/spdlog.h>

#if defined(AF_CONF_DIST)
#	define AF_DEBUG(...)
#	define AF_TRACE(...)
#else
#	define AF_DEBUG(...) ::AF::s_Logger->debug(__VA_ARGS__)
#	define AF_TRACE(...) ::AF::s_Logger->trace(__VA_ARGS__)
#endif

#define AF_INFO(...) ::AF::s_Logger->info(__VA_ARGS__)
#define AF_WARN(...) ::AF::s_Logger->warn(__VA_ARGS__)
#define AF_ERROR(...) ::AF::s_Logger->error(__VA_ARGS__)
#define AF_CRITICAL(...) ::AF::s_Logger->critical(__VA_ARGS__)
#define AF_ASSERT(statement, ...) if(!(statement)) { AF_CRITICAL(__VA_ARGS__); __debugbreak(); }

#if defined(AF_CONF_DEBUG)
#	define AF_CONF_STR "DEB"
#elif defined(AF_CONF_RELEASE)
#	define AF_CONF_STR "REL"
#elif defined(AF_CONF_DIST)
#	define AF_CONF_STR "DIST"
#else
#	error "Invalid configuration"
#endif

#if defined(AF_PLAT_WINDOWS)
#	define AF_PLAT_STR "WIN"
#else
#	error "Invalid platform"
#endif

namespace AF
{
	extern std::shared_ptr<spdlog::logger> s_Logger;

	void CreateLogger();
}