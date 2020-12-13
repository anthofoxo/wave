#pragma once

#include <memory>

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
		std::shared_ptr<State> GetState();
		void SetState(std::shared_ptr<State> state);

		void Update();
	private:
		std::shared_ptr<State> m_State = nullptr;
	};
}