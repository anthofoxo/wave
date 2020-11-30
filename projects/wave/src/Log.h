#pragma once

#include <memory>

#include <spdlog/spdlog.h>

#define AF_DEBUG(...) ::AF::s_Logger->debug(__VA_ARGS__)
#define AF_TRACE(...) ::AF::s_Logger->trace(__VA_ARGS__)
#define AF_INFO(...) ::AF::s_Logger->info(__VA_ARGS__)
#define AF_WARN(...) ::AF::s_Logger->warn(__VA_ARGS__)
#define AF_ERROR(...) ::AF::s_Logger->error(__VA_ARGS__)
#define AF_CRITICAL(...) ::AF::s_Logger->critical(__VA_ARGS__)
#define AF_ASSERT(statement, ...) if(!(statement)) { AF_CRITICAL(__VA_ARGS__); __debugbreak(); }

namespace AF
{
	extern std::shared_ptr<spdlog::logger> s_Logger;

	void CreateLogger();
}