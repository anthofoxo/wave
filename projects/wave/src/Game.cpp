#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <memory>
#include <array>
#include <vector>

#include "Debugger.h"
#include "Log.h"
#include "Application.h"
#include "Timer.h"

struct EntityManager;

class Entity
{
public:
	Entity() = default;

	virtual ~Entity() = default;

	virtual void Init()
	{
	}

	virtual void Destroy()
	{
	}

	virtual void Behaviour()
	{
	}

	virtual void Update();

	glm::vec2 m_Velocity = { 0.0f, 0.0f };
	glm::vec2 m_Position = { 0.0f, 0.0f };
	glm::vec2 m_Size = { 32.0f, 32.0f };
	glm::vec4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool m_KillMe = false;

	EntityManager* m_Manager = nullptr;
};

struct EntityManager
{
	EntityManager(AF::State* state)
		: m_State(state)
	{
	}

	void AddEntity(std::shared_ptr<Entity> entity)
	{
		entity->m_Manager = this;
		entity->Init();
		m_Entities.push_back(entity);
	}

	void RemoveEntity(std::shared_ptr<Entity> entity)
	{
		entity->m_Manager = nullptr;
		entity->Destroy();
		m_Entities.erase(std::remove(m_Entities.begin(), m_Entities.end(), entity));
	}

	void Update()
	{
		std::vector<std::shared_ptr<Entity>> theOnesThatWantKilled;

		for (int i = static_cast<int>(m_Entities.size()) - 1; i >= 0; --i)
		{
			auto entity = m_Entities[i];
			entity->Update();

			if (entity->m_KillMe)
				theOnesThatWantKilled.push_back(entity);
		}

		for (auto& entity : theOnesThatWantKilled)
			m_Entities.erase(std::remove(m_Entities.begin(), m_Entities.end(), entity));
	}

	size_t GetSize()
	{
		return m_Entities.size();
	}

	size_t GetCapacity()
	{
		return m_Entities.capacity();
	}

	std::vector<std::shared_ptr<Entity>> m_Entities;

	AF::State* m_State = nullptr;
};

void Entity::Update()
{
	auto* app = m_Manager->m_State->GetStateManager()->GetApplication();

	Behaviour();
	m_Position += m_Velocity * static_cast<float>(app->m_DeltaTime);

	app->m_Renderer.VGRP_FillRect(m_Position, m_Size, m_Color);
}

class Trail : public Entity
{
public:
	Trail(float lifetime, glm::vec2 position, glm::vec4 color)
		: m_Timer(lifetime, true)
	{
		m_Position = position;
		m_Color = color;
	}

	virtual ~Trail() = default;

	virtual void Behaviour() override
	{
		m_KillMe = m_Timer.Update(m_Manager->m_State->GetStateManager()->GetApplication()->m_DeltaTime);

		m_Color.a = 1.0f - static_cast<float>(m_Timer.PercentComplete());
	}

	AF::Timer m_Timer;
};

class MenuParticle : public Entity
{
public:
	virtual void Init() override
	{
		int direction = glm::linearRand<int>(0, 3);
		float speed = glm::linearRand<float>(300.0f, 600.0f);

		auto* app = m_Manager->m_State->GetStateManager()->GetApplication();

		m_Position.x = glm::linearRand<float>(-m_Size.x, app->m_ReferenceSize.x);
		m_Position.y = glm::linearRand<float>(-m_Size.y, app->m_ReferenceSize.y);

		switch (direction)
		{
			case 0:
				m_Velocity = { 0, 1 };
				m_Position.y = -m_Size.y;
				break;
			case 1:
				m_Velocity = { 0, -1 };
				m_Position.y = app->m_ReferenceSize.y;
				break;
			case 2:
				m_Velocity = { 1, 0 };
				m_Position.x = -m_Size.x;
				break;
			case 3:
				m_Velocity = { -1, 0 };
				m_Position.x = app->m_ReferenceSize.x;
				break;
		}

		m_Velocity *= speed;
	}

	virtual ~MenuParticle() = default;

	virtual void Behaviour() override
	{
		auto* app = m_Manager->m_State->GetStateManager()->GetApplication();

		m_Color.r = glm::linearRand<float>(0.0f, 1.0f);
		m_Color.g = glm::linearRand<float>(0.0f, 1.0f);
		m_Color.b = glm::linearRand<float>(0.0f, 1.0f);

		if (m_Position.x > app->m_ReferenceSize.x + m_Size.x * 2.0f) m_KillMe = true;
		if (m_Position.y > app->m_ReferenceSize.y + m_Size.y * 2.0f) m_KillMe = true;

		if (m_Position.x < -m_Size.x * 2.0f) m_KillMe = true;
		if (m_Position.y < -m_Size.y * 2.0f) m_KillMe = true;

		if (m_Timer.Update(m_Manager->m_State->GetStateManager()->GetApplication()->m_DeltaTime))
		{
			auto trail = std::make_shared<Trail>(0.3f, m_Position, m_Color);
			m_Manager->AddEntity(trail);
		}
	}

	AF::Timer m_Timer = AF::Timer(0.05);
};

class BasicEnemy : public Entity
{
public:
	virtual void Init() override
	{
		auto* app = m_Manager->m_State->GetStateManager()->GetApplication();

		m_Color = { 1.0f, 0.0f, 0.0f, 1.0f };

		glm::vec2 speedRange = { 100.0f, 500.0f };

		do
		{
			m_Velocity.x = glm::linearRand<float>(-speedRange[1], speedRange[1]);
			m_Velocity.y = glm::linearRand<float>(-speedRange[1], speedRange[1]);
		}
		while(glm::length(m_Velocity) < speedRange[0]);

		m_Position.x = glm::linearRand<float>(0.0f, app->m_ReferenceSize.x - m_Size.x);
		m_Position.y = glm::linearRand<float>(0.0f, app->m_ReferenceSize.y - m_Size.y);
	}

	virtual ~BasicEnemy() = default;

	virtual void Behaviour() override
	{
		auto* app = m_Manager->m_State->GetStateManager()->GetApplication();

		if (m_Position.x + m_Size.x > app->m_ReferenceSize.x)
		{
			m_Position.x = app->m_ReferenceSize.x - m_Size.x;
			m_Velocity.x *= -1.0f;
		}

		if (m_Position.y + m_Size.y > app->m_ReferenceSize.y)
		{
			m_Position.y = app->m_ReferenceSize.y - m_Size.y;
			m_Velocity.y *= -1.0f;
		}

		if (m_Position.x < 0.0f)
		{
			m_Position.x = 0.0f;
			m_Velocity.x *= -1.0f;
		}

		if (m_Position.y < 0.0f)
		{
			m_Position.y = 0.0f;
			m_Velocity.y *= -1.0f;
		}

		if (m_Timer.Update(m_Manager->m_State->GetStateManager()->GetApplication()->m_DeltaTime))
		{
			auto trail = std::make_shared<Trail>(0.3f, m_Position, m_Color);
			m_Manager->AddEntity(trail);
		}
	}

	AF::Timer m_Timer = AF::Timer(0.02);
};

class GameState : public AF::State
{
public:
	EntityManager m_EntityManager = EntityManager(this);

	GameState() = default;

	virtual ~GameState() = default;

	virtual void Update() override
	{
		auto* app = GetStateManager()->GetApplication();

		if (m_Timer.Update(app->m_DeltaTime))
			m_EntityManager.AddEntity(std::make_shared<BasicEnemy>());

		app->m_Renderer.BeginFrame(app->m_ReferenceSize);
		m_EntityManager.Update();
		app->m_Renderer.EndFrame();

		app->m_Renderer.BeginFrame(app->m_Size);
		AF::Debugger::Update();
		app->m_Renderer.EndFrame();
	}

	virtual void Attach() override
	{
	}

	virtual void Detach() override
	{
	}

	AF::Timer m_Timer = AF::Timer(5.0);
};

class MenuState : public AF::State
{
public:
	struct Button
	{
		const char* name;
		void(*onClick)();
	};

	int selectedOption = 1;
	
	virtual void Update()
	{
		auto* app = GetStateManager()->GetApplication();

		std::array<Button, 3> texts =
		{
			Button
			{
				app->m_Title,
				nullptr
			},
			Button
			{
				"Play",
				[]()
				{
					AF::GetApplication()->InvokeLater([]()
					{
						AF::GetApplication()->m_StateManager.SetState(std::make_shared<GameState>());
					});
				}
			},
			Button
			{
				"Quit",
				[]()
				{
					AF::GetApplication()->Stop();
				}
			}
		};

		app->m_Renderer.BeginFrame(app->m_ReferenceSize);
		m_EntityManager.Update();
		app->m_Renderer.EndFrame();

		app->m_Renderer.BeginFrame(app->m_Size);

		app->m_Renderer.FontFace("Roboto");

		float spacing = app->m_Size.y / (static_cast<float>(texts.size()) * 2.0f);
		float mainX = app->m_Size.x / 2.0f;

		nvgTextAlign(app->m_Renderer.m_Vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

		if (app->m_PressedKeys.find(GLFW_KEY_W) != app->m_PressedKeys.end()) --selectedOption;
		if (app->m_PressedKeys.find(GLFW_KEY_S) != app->m_PressedKeys.end()) ++selectedOption;

		if (app->m_PressedKeys.find(GLFW_KEY_SPACE) != app->m_PressedKeys.end())
		{
			texts[selectedOption].onClick();
		}

		selectedOption = glm::clamp<int>(selectedOption, 1, static_cast<int>(texts.size()) - 1);

		for (int i = 0; i < texts.size(); ++i)
		{
			float x = mainX;
			float y = spacing * static_cast<float>(i * 2 + 1);

			float b = 1.0f;

			if (i == selectedOption)
				b = 0.0f;

			if (i == 0)
			{
				app->m_Renderer.FontSize(app->ComputeFromReference(120.0f));

				app->m_Renderer.FillColor({ 1.0f, 1.0f, b, 0.5f });
				nvgText(app->m_Renderer.m_Vg, x, y, texts[i].name, nullptr);

				float offsetSize = app->ComputeFromReference(5);
				x += glm::linearRand<float>(-offsetSize, offsetSize);
				y += glm::linearRand<float>(-offsetSize, offsetSize);
			}
			else
				app->m_Renderer.FontSize(app->ComputeFromReference(60.0f));

			app->m_Renderer.FillColor({ 1.0f, 1.0f, b, 1.0f });
			nvgText(app->m_Renderer.m_Vg, x, y, texts[i].name, nullptr);
		}
		
		AF::Debugger::s_Enabled = true;
		AF::Debugger::Update();

		app->m_Renderer.EndFrame();

		m_Timer += app->m_DeltaTime;

		constexpr float timerFreq = 0.12f;

		while (m_Timer > timerFreq)
		{
			m_Timer -= timerFreq;

			auto entity = std::make_shared<MenuParticle>();
			m_EntityManager.AddEntity(entity);
		}
	}

	virtual void Attach()
	{
		
	}

	virtual void Detach()
	{

	}

	EntityManager m_EntityManager = EntityManager(this);

	double m_Timer = 0.0;
};

namespace AF
{
	Application* CreateApplication()
	{
		Application* app = new AF::Application();

		app->InvokeLater([app]()
		{
			app->m_StateManager.SetState(std::make_shared<MenuState>());
		});

		return app;
	}
}