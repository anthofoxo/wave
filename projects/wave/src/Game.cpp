#include <memory>
#include <array>
#include <vector>
#include <iostream>
#include <typeinfo>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "Debugger.h"
#include "Log.h"
#include "Application.h"
#include "Timer.h"

struct EntityManager;

enum EntityTag : uint8_t
{
	NONE, PLAYER, ENEMY, TRAIL
};

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

	virtual bool IntersectsWith(std::shared_ptr<Entity> other)
	{
		glm::vec4 a = { m_Position, m_Size };
		glm::vec4 b = { other->m_Position, other->m_Size };

		return (glm::abs((a.x + a.z / 2.0f) - (b.x + b.z / 2.0f)) * 2.0f < (a.z + b.z)) &&
			(glm::abs((a.y + a.w / 2.0f) - (b.y + b.w / 2.0f)) * 2.0f < (a.w + b.w));
	}

	glm::vec2 m_Velocity = { 0.0f, 0.0f };
	glm::vec2 m_Position = { 0.0f, 0.0f };
	glm::vec2 m_Size = { 32.0f, 32.0f };
	glm::vec4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool m_KillMe = false;
	EntityTag m_Tag = EntityTag::NONE;

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
	auto* app = AF::GetApplication();

	Behaviour();

	app->m_Renderer.VGRP_FillRect(m_Position, m_Size, m_Color);
}

class Trail : public Entity
{
public:
	Trail(float lifetime, glm::vec2 position, glm::vec2 size, glm::vec4 color)
		: m_Timer(lifetime, true)
	{
		m_Position = position;
		m_Color = color;
		m_Size = size;
		m_Tag = EntityTag::TRAIL;
	}

	virtual ~Trail() = default;

	virtual void Behaviour() override
	{
		m_KillMe = m_Timer.Update(static_cast<float>(AF::GetApplication()->m_DeltaTime));

		m_Color.a = 1.0f - m_Timer.PercentComplete();
	}

	AF::Timer<float> m_Timer;
};

class MenuParticle : public Entity
{
public:
	virtual void Init() override
	{
		int direction = glm::linearRand<int>(0, 3);
		float speed = glm::linearRand<float>(300.0f, 600.0f);

		auto* app = AF::GetApplication();

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
		auto* app = AF::GetApplication();

		m_Color.r = glm::linearRand<float>(0.0f, 1.0f);
		m_Color.g = glm::linearRand<float>(0.0f, 1.0f);
		m_Color.b = glm::linearRand<float>(0.0f, 1.0f);

		m_Position += m_Velocity * static_cast<float>(app->m_DeltaTime);

		if (m_Position.x > app->m_ReferenceSize.x + m_Size.x * 2.0f) m_KillMe = true;
		if (m_Position.y > app->m_ReferenceSize.y + m_Size.y * 2.0f) m_KillMe = true;

		if (m_Position.x < -m_Size.x * 2.0f) m_KillMe = true;
		if (m_Position.y < -m_Size.y * 2.0f) m_KillMe = true;

		if (m_Timer.Update(static_cast<float>(app->m_DeltaTime)))
		{
			auto trail = std::make_shared<Trail>(0.3f, m_Position, m_Size, m_Color);
			m_Manager->AddEntity(trail);
		}
	}

	AF::Timer<float> m_Timer = AF::Timer<float>(0.05f);
};

class BasicEnemy : public Entity
{
public:
	virtual void Init() override
	{
		m_Tag = EntityTag::ENEMY;

		auto* app = AF::GetApplication();

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
		auto* app = AF::GetApplication();

		m_Position += m_Velocity * static_cast<float>(app->m_DeltaTime);

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

		if (m_Timer.Update(static_cast<float>(app->m_DeltaTime)))
		{
			auto trail = std::make_shared<Trail>(0.2f, m_Position, m_Size, m_Color);
			m_Manager->AddEntity(trail);
		}
	}

	AF::Timer<float> m_Timer = AF::Timer<float>(0.01f);
};

class FastEnemy : public Entity
{
public:
	virtual void Init() override
	{
		m_Tag = EntityTag::ENEMY;

		auto* app = AF::GetApplication();

		m_Color = { 0.0f, 0.5f, 1.0f, 1.0f };

		glm::vec2 speedRange = { 500.0f, 1000.0f };

		do
		{
			m_Velocity.x = glm::linearRand<float>(-speedRange[1], speedRange[1]);
			m_Velocity.y = glm::linearRand<float>(-speedRange[1], speedRange[1]);
		} while (glm::length(m_Velocity) < speedRange[0]);

		m_Position.x = glm::linearRand<float>(0.0f, app->m_ReferenceSize.x - m_Size.x);
		m_Position.y = glm::linearRand<float>(0.0f, app->m_ReferenceSize.y - m_Size.y);
	}

	virtual ~FastEnemy() = default;

	virtual void Behaviour() override
	{
		auto* app = AF::GetApplication();

		m_Position += m_Velocity * static_cast<float>(app->m_DeltaTime);

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

		if (m_Timer.Update(static_cast<float>(app->m_DeltaTime)))
		{
			auto trail = std::make_shared<Trail>(0.2f, m_Position, m_Size, m_Color);
			m_Manager->AddEntity(trail);
		}
	}

	AF::Timer<float> m_Timer = AF::Timer<float>(0.01f);
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

	virtual void Update();
	virtual void Attach();
	virtual void Detach();

	EntityManager m_EntityManager = EntityManager(this);

	AF::Timer<float> m_Timer = AF::Timer<float>(0.12f);
	AF::Timer<float> m_FadeTimer = AF::Timer<float>(0.5f, true);
};

class Player : public Entity
{
public:
	virtual void Init() override
	{
		m_Tag = EntityTag::PLAYER;

		auto* app = AF::GetApplication();

		m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };

		m_Position.x = (app->m_ReferenceSize.x - m_Size.x) / 2.0f;
		m_Position.y = (app->m_ReferenceSize.y - m_Size.y) / 2.0f;
	}

	virtual ~Player() = default;

	virtual void Behaviour() override
	{
		auto* app = AF::GetApplication();

		m_Velocity = { 0.0f, 0.0f };

		if (app->m_Keys.find(GLFW_KEY_A) != app->m_Keys.end()) --m_Velocity.x;
		if (app->m_Keys.find(GLFW_KEY_D) != app->m_Keys.end()) ++m_Velocity.x;
		if (app->m_Keys.find(GLFW_KEY_W) != app->m_Keys.end()) --m_Velocity.y;
		if (app->m_Keys.find(GLFW_KEY_S) != app->m_Keys.end()) ++m_Velocity.y;

		m_Velocity *= 500.0f;

		m_Position += m_Velocity * static_cast<float>(app->m_DeltaTime);

		m_Position = glm::clamp(m_Position, { 0.0f, 0.0f }, app->m_ReferenceSize - m_Size);

		if (m_Timer.Update(static_cast<float>(app->m_DeltaTime)))
		{
			auto trail = std::make_shared<Trail>(0.2f, m_Position, m_Size, m_Color);
			m_Manager->AddEntity(trail);
		}

		for (auto entity : m_Manager->m_Entities)
		{
			if (entity->m_Tag != EntityTag::ENEMY) continue;

			if (IntersectsWith(entity))
			{
				currentHealth -= (entity->m_Size.x * 3.0f) * app->m_DeltaTime;
			}
		}

		if (currentHealth <= 0.0f)
		{
			app->InvokeLater([]()
			{
				AF::GetApplication()->m_StateManager.SetState(std::make_shared<MenuState>());
			});
		}

		currentHealth += app->m_DeltaTime * 5.0f;
		currentHealth = glm::clamp(currentHealth, 0.0f, maxHealth);

		float width = 200.0f;
		float healthWidth = currentHealth / maxHealth * width;

		app->m_Renderer.VGRP_FillRect(m_Position + glm::vec2{ 100.0f + healthWidth, 100.0f }, { width - healthWidth, 20.0f }, { 1.0f, 1.0f, 1.0f, 0.5f });
		app->m_Renderer.VGRP_FillRect(m_Position + glm::vec2{ 100.0f, 100.0f }, { healthWidth, 20.0f }, { 1.0f, 0.0f, 0.0f, 0.75f });
	}

	AF::Timer<float> m_Timer = AF::Timer<float>(0.01f);
	float maxHealth = 100.0f;
	float currentHealth = maxHealth;
};

class GameState : public AF::State
{
public:
	EntityManager m_EntityManager = EntityManager(this);

	GameState() = default;

	virtual ~GameState() = default;

	virtual void Update() override
	{
		auto* app = AF::GetApplication();

		if (m_Timer.Update(static_cast<float>(app->m_DeltaTime)))
		{
			++m_CurrentLevel;

			

			if (m_CurrentLevel == 4)
			{
				std::vector<std::shared_ptr<Entity>> currentEntities;

				for (auto entity : m_EntityManager.m_Entities)
				{
					if(entity->m_Tag == EntityTag::ENEMY)
						currentEntities.push_back(entity);
				}

				for (auto entity : currentEntities)
				{
					m_EntityManager.RemoveEntity(entity);
				}

				for (auto entity : currentEntities)
				{
					auto enemy1 = std::make_shared<BasicEnemy>();
					auto enemy2 = std::make_shared<BasicEnemy>();
					auto enemy3 = std::make_shared<BasicEnemy>();
					auto enemy4 = std::make_shared<BasicEnemy>();

					m_EntityManager.AddEntity(enemy1);
					m_EntityManager.AddEntity(enemy2);
					m_EntityManager.AddEntity(enemy3);
					m_EntityManager.AddEntity(enemy4);

					enemy1->m_Size = entity->m_Size * 0.5f;
					enemy2->m_Size = entity->m_Size * 0.5f;
					enemy3->m_Size = entity->m_Size * 0.5f;
					enemy4->m_Size = entity->m_Size * 0.5f;

					enemy1->m_Position = entity->m_Position + glm::vec2(0.0f, 0.0f);
					enemy2->m_Position = entity->m_Position + glm::vec2(0.0f, entity->m_Size.y * 0.25f);
					enemy3->m_Position = entity->m_Position + glm::vec2(entity->m_Size.x * 0.25f, 0.0f);
					enemy4->m_Position = entity->m_Position + glm::vec2(entity->m_Size.x * 0.25f, entity->m_Size.y * 0.25f);

					enemy1->m_Velocity = entity->m_Velocity * 0.8f;
					enemy2->m_Velocity = entity->m_Velocity * 0.8f;
					enemy3->m_Velocity = entity->m_Velocity * 0.8f;
					enemy4->m_Velocity = entity->m_Velocity * 0.8f;

					float min = 32.0f;

					enemy1->m_Velocity += glm::vec2{ glm::linearRand<float>(-min, min), glm::linearRand<float>(-min, min) };
					enemy2->m_Velocity += glm::vec2{ glm::linearRand<float>(-min, min), glm::linearRand<float>(-min, min) };
					enemy3->m_Velocity += glm::vec2{ glm::linearRand<float>(-min, min), glm::linearRand<float>(-min, min) };
					enemy4->m_Velocity += glm::vec2{ glm::linearRand<float>(-min, min), glm::linearRand<float>(-min, min) };
					
				}

			}
			else if(m_CurrentLevel > 5)
			{
				m_EntityManager.AddEntity(std::make_shared<FastEnemy>());
			}
			else
			{
				m_EntityManager.AddEntity(std::make_shared<BasicEnemy>());
			}
		}

		app->m_Renderer.BeginFrame(app->m_ReferenceSize);
		m_EntityManager.Update();
		app->m_Renderer.EndFrame();

		app->m_Renderer.BeginFrame(app->m_Size);
		AF::Debugger::Update();
		app->m_Renderer.EndFrame();
	}

	virtual void Attach() override
	{
		m_EntityManager.AddEntity(std::make_shared<Player>());
	}

	virtual void Detach() override
	{
	}

	AF::Timer<float> m_Timer = AF::Timer<float>(5.0f);
	int m_CurrentLevel = 0;
};

void MenuState::Update()
{
	auto* app = AF::GetApplication();

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
		
	AF::Debugger::Update();


	app->m_Renderer.EndFrame();

	if (m_Timer.Update(static_cast<float>(app->m_DeltaTime)))
	{
		auto entity = std::make_shared<MenuParticle>();
		m_EntityManager.AddEntity(entity);
	}


}

void MenuState::Attach()
{	
}

void MenuState::Detach()
{
}



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