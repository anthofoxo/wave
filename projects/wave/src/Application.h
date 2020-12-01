#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <nanovg.h>

#include "State.h"

namespace AF
{
	class Application final
	{
	public:
		void Start();
		void Stop();

		void EarlyInit();
		void Init();
		void Destroy();

		void Update();

		void Resize(glm::vec2 size);
		float ComputeFromReference(float input);

		bool m_Running = false;
		glm::vec2 m_Size = { 1280, 720 };
		glm::vec2 m_ReferenceSize = { 1280, 720 };
		const char* m_Title = "Wave";
		double m_DeltaTime = 0.0f;
		NVGcontext* m_Ctx = nullptr;
		GLFWwindow* m_Window = nullptr;

		StateManager m_StateManager{this};
	};
}