#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>
#include <glm/glm.hpp>

int main(int, char**)
{
	glm::vec2 size = {1280, 720};

	std::shared_ptr<spdlog::logger> console = spdlog::stdout_color_mt("wave");

	console->set_level(spdlog::level::debug);

	console->info("Arooooo");

	glfwInit();

	GLFWwindow* window = glfwCreateWindow(800, 600, "Wave", nullptr, nullptr);

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	NVGcontext* vg = nvgCreateGL3(/*NVG_ANTIALIAS | NVG_STENCIL_STROKES*/ 0);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(1, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		nvgBeginFrame(vg, 800, 600, 1);

		nvgBeginPath(vg);
		nvgRect(vg, 10, 10, 10, 10);
		nvgFillColor(vg, nvgRGBA(0, 255, 0, 255));
		nvgFill(vg);

		nvgEndFrame(vg);

		glfwSwapBuffers(window);
	}

	nvgDeleteGL3(vg);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}