#include "Application.h"

#define NANOVG_GL3_IMPLEMENTATION

#include <thread>

#include <glad/glad.h>
#include <nanovg.h>
#include <nanovg_gl.h>
#include <glm/gtc/random.hpp>

#include "Log.h"

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
		});

		while (m_Running)
		{
			glfwPollEvents();
		}

		thread.join();

		Destroy();

		AF_INFO("Stopped application");
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
		glfwSetWindowAspectRatio(m_Window, m_Size.x, m_Size.y);

		AF_TRACE("Setting callbacks");

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			static_cast<Application*>(glfwGetWindowUserPointer(window))->Stop();
		});

		glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			static_cast<Application*>(glfwGetWindowUserPointer(window))->Resize({ width, height });
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
		m_Ctx = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
		AF_ASSERT(m_Ctx, "Failed to initialize nanovg");

		AF_TRACE("Loading nanovg font");
		int result = nvgCreateFont(m_Ctx, "Roboto", "res/Roboto-Medium.ttf");
		AF_ASSERT(result != -1, "Failed to load font");
	}

	void Application::Update()
	{
		glViewport(0, 0, static_cast<int>(m_Size.x), static_cast<int>(m_Size.y));
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		m_StateManager.Update();

		glfwSwapBuffers(m_Window);
	}

	void Application::Destroy()
	{
		AF_DEBUG("Destroying nanovg");
		nvgDeleteGL3(m_Ctx);
		m_Ctx = nullptr;

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