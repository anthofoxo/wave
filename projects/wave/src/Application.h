#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <nanovg.h>
#include <vector>
#include <functional>
#include <unordered_set>
#include <mutex>

#include "State.h"
#include "Renderer.h"

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

		void InvokeLater(const std::function<void()>& function);

		std::vector<std::function<void()>> m_Futures;

		std::unordered_set<int> m_Keys;
		std::unordered_set<int> m_PressedKeys;

		std::mutex m_FutureMutex;

		bool m_Running = false;
		glm::vec2 m_Size = { 1280, 720 };
		glm::vec2 m_ReferenceSize = { 1280, 720 };
		const char* m_Title = "Wave";
		double m_DeltaTime = 0.0f;
		Renderer m_Renderer;
		GLFWwindow* m_Window = nullptr;

		StateManager m_StateManager{this};
	};
}