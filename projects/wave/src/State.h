#pragma once

#include <memory>

// Application.h
namespace AF
{
	class Application;
}

namespace AF
{
	class StateManager;

	class State
	{
		friend class StateManager;
	public:
		StateManager* GetStateManager();

		virtual void Update() = 0;
	private:
		virtual void Attach() = 0;
		virtual void Detach() = 0;

		StateManager* m_StateManager = nullptr;
	};

	class StateManager final
	{
	public:
		StateManager(Application* application);

		Application* GetApplication();

		std::shared_ptr<State> GetState();
		void SetState(std::shared_ptr<State> state);

		void Update();
	private:
		std::shared_ptr<State> m_State = nullptr;
		Application* m_Application = nullptr;
	};
}