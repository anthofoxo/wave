#include "Application.h"

#define NANOVG_GL3_IMPLEMENTATION

#include <thread>

#include <glad/glad.h>
#include <nanovg.h>
#include <nanovg_gl.h>
#include <glm/gtc/random.hpp>
#include <portaudio.h>

#define STB_VORBIS_HEADER_ONLY
#include "vendor/stb_vorbis.c"

#include "Log.h"
#include "Debugger.h"

namespace AF
{
	void Application::Start()
	{
		if (m_Running) return;
		m_Running = true;

		AF_INFO("Starting application");

		EarlyInit();

		std::thread thread = std::thread([&]()
		{
			PaError err = Pa_Initialize();
			if (err != paNoError)
				printf("PortAudio error: %s\n", Pa_GetErrorText(err));

			PaStream* stream;
			
		

			short* decoded;
			int channels, rate, len;
			len = stb_vorbis_decode_filename("res/music.ogg", &channels, &rate, &decoded);


			struct TestData
			{
				short* data;
				size_t position;
				int max_position;
			};

			TestData data = { decoded, 0, 44100 * 2 * 5 };

			err = Pa_OpenDefaultStream(&stream, 0, 2, paInt16, 44100, 256, [](
				const void* input,
				void* output,
				unsigned long frameCount,
				const PaStreamCallbackTimeInfo* timeInfo,
				PaStreamCallbackFlags statusFlags,
				void* userData
				) -> int
			{
				short* out = (short*) output;
				TestData* data = (TestData*) userData;

				for (int i = 0; i < 256; i++)
				{
					*out++ = data->data[data->position++];
					*out++ = data->data[data->position++];
				}

				if (data->position >= data->max_position) data->position -= data->max_position;

				return 0;
			}, &data);

			if (err != paNoError)
				printf("PortAudio error: %s\n", Pa_GetErrorText(err));

			err = Pa_StartStream(stream);
			if (err != paNoError)
				printf("PortAudio error: %s\n", Pa_GetErrorText(err));

			Init();

			double lastTime = glfwGetTime();
			double currentTime;

			AF_INFO("Starting gameloop");

			while (m_Running)
			{
				currentTime = glfwGetTime();
				m_DeltaTime = currentTime - lastTime;
				lastTime = currentTime;

				Update();
			}

			err = Pa_StopStream(stream);
			if (err != paNoError)
				printf("PortAudio error: %s\n", Pa_GetErrorText(err));

			err = Pa_Terminate();
			if (err != paNoError)
				printf("PortAudio error: %s\n", Pa_GetErrorText(err));
		});

		while (m_Running)
		{
			glfwPollEvents();
		}

		thread.join();

		Destroy();

		AF_INFO("Stopped application");
	}

	void Application::InvokeLater(const std::function<void()>& function)
	{
		m_FutureMutex.lock();

		m_Futures.push_back(function);

		m_FutureMutex.unlock();
	}

	void Application::Stop()
	{
		AF_INFO("Stopping application");
		m_Running = false;
	}

	void Application::EarlyInit()
	{
		AF_DEBUG("Starting EarlyInit");

		AF_TRACE("Initializing glfw");
		AF_ASSERT(glfwInit(), "Failed to initialze glfw");

		AF_TRACE("Setting window hints");
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

		AF_TRACE("Creating window");
		m_Window = glfwCreateWindow(static_cast<int>(m_Size.x), static_cast<int>(m_Size.y), m_Title, nullptr, nullptr);
		AF_ASSERT(m_Window, "Failed to create window");

		glfwSetWindowUserPointer(m_Window, this);

		AF_TRACE("Enforcing aspect ratio");
		glfwSetWindowAspectRatio(m_Window, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y));

		AF_TRACE("Setting callbacks");

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			static_cast<Application*>(glfwGetWindowUserPointer(window))->Stop();
		});

		glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			static_cast<Application*>(glfwGetWindowUserPointer(window))->Resize({ width, height });
		});
		
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
					app->InvokeLater([key, app]()
					{
						app->m_Keys.insert(key);
						app->m_PressedKeys.insert(key);
					});
					break;
				case GLFW_RELEASE:
					app->InvokeLater([key, app]()
					{
						app->m_Keys.erase(key);
					});
					break;
			}
		});
	}

	void Application::Init()
	{
		AF_DEBUG("Starting Init");

		AF_TRACE("Loading opengl");
		glfwMakeContextCurrent(m_Window);

		AF_TRACE("Setting vsync");
		glfwSwapInterval(1);

		AF_ASSERT(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress), "Failed to load opengl");

		AF_TRACE("Loading nanovg");
		m_Renderer.m_Vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
		AF_ASSERT(m_Renderer.m_Vg, "Failed to initialize nanovg");

		AF_TRACE("Loading nanovg font");
		int result = nvgCreateFont(m_Renderer.m_Vg, "Roboto", "res/Roboto-Medium.ttf");
		AF_ASSERT(result != -1, "Failed to load font");

		AF_TRACE("Attaching ingame debugger");

		struct DebugGeneralInfo : public AF::DebuggerSection
		{
			DebugGeneralInfo()
			{
				m_Title = "General Information";

				const char* vendor = (const char*) glGetString(GL_VENDOR);
				const char* renderer = (const char*) glGetString(GL_RENDERER);
				const char* version = (const char*) glGetString(GL_VERSION);
				const char* glslVersion = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);

				std::string string = fmt::format("{} {} {} {}", AF_PLAT_STR, AF_CONF_STR, __DATE__, __TIME__);
				m_Content.push_back(std::make_pair("Game Version", std::move(string)));

				m_Content.push_back(std::make_pair("GPU Vendor", vendor));
				m_Content.push_back(std::make_pair("GPU Renderer", renderer));
				m_Content.push_back(std::make_pair("GPU Version", version));
				m_Content.push_back(std::make_pair("GPU GLSL Version", glslVersion));
			}

			virtual ~DebugGeneralInfo() = default;
		}
		generalDebugger;

		AF::Debugger::s_Sections.push_back(generalDebugger);
	}

	void Application::Update()
	{
		glViewport(0, 0, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y));
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		m_StateManager.Update();

		glfwSwapBuffers(m_Window);

		m_PressedKeys.clear();

		while (true)
		{
			m_FutureMutex.lock();
			size_t size = m_Futures.size();
			m_FutureMutex.unlock();

			if (size == 0) break;

			m_FutureMutex.lock();
			auto future = m_Futures[m_Futures.size() - 1];
			m_Futures.pop_back();
			m_FutureMutex.unlock();

			future();
		}
	}

	void Application::Destroy()
	{
		AF_DEBUG("Destroying nanovg");
		nvgDeleteGL3(m_Renderer.m_Vg);
		m_Renderer.m_Vg = nullptr;

		AF_DEBUG("Destroying window");
		glfwDestroyWindow(m_Window);
		m_Window = nullptr;

		AF_DEBUG("Destroying glfw");
		glfwTerminate();
	}

	void Application::Resize(glm::vec2 size)
	{
		AF_TRACE("Resize triggered: {}, {}", size.x, size.y);

		m_Size = size;
	}

	float Application::ComputeFromReference(float input)
	{
		return input / glm::length(m_ReferenceSize) * glm::length(m_Size);
	}
}