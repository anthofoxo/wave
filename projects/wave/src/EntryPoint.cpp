#include "Application.h"
#include "Log.h"

#define AF_MAIN() int main(int, char**)
#if defined(AF_PLAT_WINDOWS) && defined(AF_CONF_DIST)
#	undef AF_MAIN
#	define AF_MAIN() int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#endif

namespace AF
{
	Application* s_Application;

	Application* GetApplication()
	{
		return s_Application;
	}
}

AF_MAIN()
{
	AF::CreateLogger();

	AF_INFO("Started");

	AF::s_Application = AF::CreateApplication();
	AF::s_Application->Start();
	delete AF::s_Application;
	AF::s_Application = nullptr;

	AF_INFO("Stopped");
	return 0;
}