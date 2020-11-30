#include "State.h"

namespace AF
{
	StateManager* State::GetStateManager()
	{
		return m_StateManager;
	}

	StateManager::StateManager(Application* application)
		: m_Application(application)
	{
	}

	Application* StateManager::GetApplication()
	{
		return m_Application;
	}

	std::shared_ptr<State> StateManager::GetState()
	{
		return m_State;
	}

	void StateManager::SetState(std::shared_ptr<State> state)
	{
		if (m_State)
		{
			m_State->Detach();
			m_State->m_StateManager = nullptr;
		}

		m_State = state;

		if (m_State)
		{
			m_State->m_StateManager = this;
			m_State->Attach();
		}
	}

	void StateManager::Update()
	{
		if (m_State)
			m_State->Update();
	}
}