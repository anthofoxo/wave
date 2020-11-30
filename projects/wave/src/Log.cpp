#include "Log.h"

namespace AF
{
	std::shared_ptr<spdlog::logger> s_Logger;

	void CreateLogger()
	{
		s_Logger = spdlog::stdout_color_mt("Wave");
		s_Logger->set_level(spdlog::level::debug);
	}
}