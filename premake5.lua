workspace "wave"
	architecture "x64"
	startproject "wave"
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

bindir = "bin/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}-%{prj.name}/"
intdir = "bin-int/%{cfg.system}-%{cfg.buildcfg}-%{cfg.architecture}-%{prj.name}/"
prjroot = "projects/%{prj.name}/"

includes = {}
includes["glad"] = "projects/glad/include/"
includes["glfw"] = "projects/glfw/include/"
includes["glm"] = "projects/glm/"
includes["spdlog"] = "projects/spdlog/include/"
includes["nanovg"] = "projects/nanovg/src/"
includes["portaudio"] = "projects/portaudio/include/"
includes["incbin"] = "projects/incbin/"

group "deps"

project "portaudio"
	location (prjroot)
	kind "StaticLib"
	language "C"
	
	targetdir (bindir)
	objdir (intdir)
	
	files
	{
		prjroot .. "src/common/**.c",
		prjroot .. "src/os/win/**.c",
		prjroot .. "src/hostapi/wmme/**.c",
	}
	
	includedirs
	{
		prjroot .. "include",
		prjroot .. "src/common"
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"PA_USE_WMME"
	}
	
	filter "configurations:Debug"
		symbols "On"
	
	filter "configurations:Release"
		optimize "On"
		
	filter "configurations:Dist"
		optimize "On"
		
	filter "system:windows"
		staticruntime "On"

project "glad"
	location (prjroot)
	kind "StaticLib"
	language "C"
	
	targetdir (bindir)
	objdir (intdir)

	files
	{
		prjroot .. "src/glad.c"
	}
	
	includedirs
	{
		prjroot .. "include"
	}
	
	filter "configurations:Debug"
		symbols "On"
	
	filter "configurations:Release"
		optimize "On"
		
	filter "configurations:Dist"
		optimize "On"
		
	filter "system:windows"
		staticruntime "On"

project "glfw"
	location (prjroot)
	kind "StaticLib"
	language "C"
	
	targetdir (bindir)
	objdir (intdir)

	files
	{
		prjroot .. "src/context.c",
		prjroot .. "src/egl_context.c",
		prjroot .. "src/egl_context.h",
		prjroot .. "src/init.c",
		prjroot .. "src/input.c",
		prjroot .. "src/internal.h",
		prjroot .. "src/monitor.c",
		prjroot .. "src/osmesa_context.c",
		prjroot .. "src/osmesa_context.h",
		prjroot .. "src/vulkan.c",
		prjroot .. "src/wgl_context.c",
		prjroot .. "src/wgl_context.h",
		prjroot .. "src/window.c",
		
		prjroot .. "src/win32_init.c",
		prjroot .. "src/win32_joystick.c",
		prjroot .. "src/win32_joystick.h",
		prjroot .. "src/win32_monitor.c",
		prjroot .. "src/win32_platform.h",
		prjroot .. "src/win32_thread.c",
		prjroot .. "src/win32_time.c",
		prjroot .. "src/win32_window.c",
	}
	
	includedirs
	{
		prjroot .. "include/"
	}
	
	defines
	{
		"_GLFW_WIN32",
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	filter "configurations:Debug"
		symbols "On"
		
	filter "configurations:Release"
		optimize "On"
		
	filter "configurations:Dist"
		optimize "On"
		
	filter "system:windows"
		staticruntime "On"
project "glm"
	location (prjroot)
	kind "StaticLib"
	language "C"
	
	targetdir (bindir)
	objdir (intdir)
	
	includedirs
	{
		prjroot .. "glm/"
	}
	
	filter "configurations:Debug"
		symbols "On"
		
	filter "configurations:Release"
		optimize "On"
		
	filter "configurations:Dist"
		optimize "On"
		
	filter "system:windows"
		staticruntime "On"

project "spdlog"
	location (prjroot)
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	targetdir (bindir)
	objdir (intdir)

	files
	{
		prjroot .. "src/*.cpp"
	}

	includedirs
	{
		prjroot .. "include/"
	}

	defines
	{
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT"
	}
	
	filter "configurations:Debug"
		symbols "On"
	
	filter "configurations:Release"
		optimize "On"
		
	filter "configurations:Dist"
		optimize "On"
		
	filter "system:windows"
		staticruntime "On"
		
project "nanovg"
	location (prjroot)
	kind "StaticLib"
	language "C"

	targetdir (bindir)
	objdir (intdir)

	files
	{
		prjroot .. "src/*.c"
	}

	includedirs
	{
		prjroot .. "src"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	filter "configurations:Debug"
		symbols "On"
		defines { "DEBUG" }
		warnings "Extra"
	
	filter "configurations:Release"
		optimize "On"
		defines { "NDEBUG" }
		warnings "Extra"
		
	filter "configurations:Dist"
		optimize "On"
		
	filter "system:windows"
		staticruntime "On"

project "incbin"
	location (prjroot)
	kind "ConsoleApp"
	language "C"

	targetdir "bin/%{cfg.system}-%{cfg.architecture}-%{prj.name}/"
	objdir "bin-int/%{cfg.system}-%{cfg.architecture}-%{prj.name}/"

	files
	{
		prjroot .. "incbin.c"
	}

	includedirs
	{
	}

	defines
	{
	}
	
	filter "configurations:Debug"
		symbols "On"
		defines { "DEBUG" }
		warnings "Extra"
	
	filter "configurations:Release"
		optimize "On"
		defines { "NDEBUG" }
		warnings "Extra"
		
	filter "configurations:Dist"
		optimize "On"
		
	filter "system:windows"
		staticruntime "On"

group ""

project "wave"
	location (prjroot)
	
	filter "configurations:Debug"
		kind "ConsoleApp"
	filter "configurations:Release"
		kind "ConsoleApp"
	filter "configurations:Dist"
		kind "WindowedApp"
	filter {}
	
	language "C++"
	cppdialect "C++17"
	
	targetdir (bindir)
	objdir (intdir)

	files
	{
		prjroot .. "src/**.h",
		prjroot .. "src/**.cpp",
		prjroot .. "src/**.c",
		
		-- This file is generated from a prebuildcommand
		prjroot .. "src/ResourcesWin.c"
	}
	
	includedirs
	{
		includes["glad"],
		includes["glfw"],
		includes["glm"],
		includes["spdlog"],
		includes["nanovg"],
		includes["portaudio"],
		includes["incbin"]
	}
	
	defines
	{
		"GLFW_INCLUDE_NONE",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT"
	}
	
	links
	{
		"glad",
		"glfw",
		"nanovg",
		"portaudio"
	}
		
	filter "system:windows"
		staticruntime "On"
		systemversion "latest"
		
		defines
		{
			"AF_PLAT_WINDOWS"
		}

	filter "configurations:Debug"
		defines
		{
			"AF_CONF_DEBUG"
		}
		symbols "On"

	filter "configurations:Release"
		defines
		{
			"AF_CONF_RELEASE"
		}
		optimize "On"
		
	filter "configurations:Dist"
		defines
		{
			"AF_CONF_DIST"
		}
		optimize "On"