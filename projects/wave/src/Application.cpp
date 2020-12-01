#include "Application.h"

#define NANOVG_GL3_IMPLEMENTATION

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

		Init();

		double lastTime = glfwGetTime();
		double currentTime;

		while (m_Running)
		{
			currentTime = glfwGetTime();
			m_DeltaTime = currentTime - lastTime;
			lastTime = currentTime;

			EarlyUpdate();
			Update();
		}

		Destroy();
	}

	void Application::Stop()
	{
		m_Running = false;
	}

	void Application::Init()
	{
		AF_ASSERT(glfwInit(), "Failed to initialze glfw");

		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

		m_Window = glfwCreateWindow(static_cast<int>(m_Size.x), static_cast<int>(m_Size.y), m_Title, nullptr, nullptr);
		AF_ASSERT(m_Window, "Failed to create window");

		// glfwSetWindowAspectRatio(m_Window, m_Size.x, m_Size.y);

		glfwSetWindowUserPointer(m_Window, this);

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			static_cast<Application*>(glfwGetWindowUserPointer(window))->Stop();
		});

		glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			static_cast<Application*>(glfwGetWindowUserPointer(window))->Resize({ width, height });
		});

		glfwSetWindowRefreshCallback(m_Window, [](GLFWwindow* window)
		{
			static_cast<Application*>(glfwGetWindowUserPointer(window))->Update();
		});

		glfwMakeContextCurrent(m_Window);
		glfwSwapInterval(1);

		AF_ASSERT(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress), "Failed to load opengl");

		m_Ctx = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
		AF_ASSERT(m_Ctx, "Failed to initialize nanovg");

		int result = nvgCreateFont(m_Ctx, "Roboto", "res/Roboto-Medium.ttf");
		AF_ASSERT(result != -1, "Failed to load font");
	}

	void Application::EarlyUpdate()
	{
		glfwPollEvents();
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
		nvgDeleteGL3(m_Ctx);
		m_Ctx = nullptr;

		glfwDestroyWindow(m_Window);
		m_Window = nullptr;

		glfwTerminate();
	}

	void Application::Resize(glm::vec2 size)
	{
		m_Size = size;
	}

	float Application::ComputeFromReference(float input)
	{
		return input / glm::length(m_ReferenceSize) * glm::length(m_Size);
	}
}