#include <GLFW/glfw3.h>
#include <glad/glad.h>


#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <memory>
#include <vector>

#include "Log.h"
#include "Application.h"

class Entity
{
public:
	Entity()
	{
		int direction = glm::linearRand<int>(0, 3);
		float speed = glm::linearRand<float>(300.0f, 600.0f);

		m_Position.x = glm::linearRand<float>(-m_Size.x, 1280.0f);
		m_Position.y = glm::linearRand<float>(-m_Size.y, 720.0f);

		switch (direction)
		{
			case 0:
				m_Velocity = { 0, 1 };
				m_Position.y = -m_Size.y;
				break;
			case 1:
				m_Velocity = { 0, -1 };
				m_Position.y = 720.0f;
				break;
			case 2:
				m_Velocity = { 1, 0 };
				m_Position.x = -m_Size.x;
				break;
			case 3:
				m_Velocity = { -1, 0 };
				m_Position.x = 1280.0f;
				break;
		}

		m_Velocity *= speed;
	}

	virtual ~Entity() = default;

	virtual void Behaviour(AF::State& state);

	virtual void Update(AF::State& state);

	glm::vec2 m_Velocity = { 0.0f, 0.0f };
	glm::vec2 m_Position = { 0.0f, 0.0f };
	glm::vec2 m_Size = { 32.0f, 32.0f };
	glm::vec4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool m_KillMe = false;
};

void Entity::Behaviour(AF::State& state)
{
	m_Color.r = glm::linearRand<float>(0.0f, 1.0f);
	m_Color.g = glm::linearRand<float>(0.0f, 1.0f);
	m_Color.b = glm::linearRand<float>(0.0f, 1.0f);

	m_Position += m_Velocity * static_cast<float>(state.GetStateManager()->GetApplication()->m_DeltaTime);

	if (m_Position.x > 1280.0f + m_Size.x * 2.0f) m_KillMe = true;
	if (m_Position.y > 720.0f + m_Size.y * 2.0f) m_KillMe = true;

	if (m_Position.x < -m_Size.x * 2.0f) m_KillMe = true;
	if (m_Position.y < -m_Size.y * 2.0f) m_KillMe = true;
}

void Entity::Update(AF::State& state)
{
	auto* app = state.GetStateManager()->GetApplication();

	Behaviour(state);

	nvgBeginPath(app->m_Ctx);
	nvgRect(app->m_Ctx, m_Position.x, m_Position.y, m_Size.x, m_Size.y);
	nvgFillColor(app->m_Ctx, *(NVGcolor*) &m_Color);
	nvgFill(app->m_Ctx);
}

class MenuState : public AF::State
{
public:
	virtual void Update()
	{
		auto* app = GetStateManager()->GetApplication();

		nvgBeginFrame(app->m_Ctx, app->m_ReferenceSize.x, app->m_ReferenceSize.y, 1);

		{
			std::vector<std::shared_ptr<Entity>> theOnesThatWantKilled;

			for (auto& entity : m_Entities)
			{
				entity->Update(*this);

				if (entity->m_KillMe)
					theOnesThatWantKilled.push_back(entity);
			}

			for (auto& entity : theOnesThatWantKilled)
			{
				m_Entities.erase(std::remove(m_Entities.begin(), m_Entities.end(), entity));
			}
		}

		nvgEndFrame(app->m_Ctx);

		nvgBeginFrame(app->m_Ctx, app->m_Size.x, app->m_Size.y, 1);

		nvgFontFace(app->m_Ctx, "Roboto");
		nvgFontSize(app->m_Ctx, app->ComputeFromReference(72.0f));

		nvgTextAlign(app->m_Ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

		float offsetSize = app->ComputeFromReference(5);
		float xOffset = glm::linearRand<float>(-offsetSize, offsetSize);
		float yOffset = glm::linearRand<float>(-offsetSize, offsetSize);

		float titleX = app->m_Size.x / 2.0f;
		float titleY = app->m_Size.y / 4.0f;

		nvgFillColor(app->m_Ctx, { 1.0f, 1.0f, 1.0f, 0.5f });
		nvgText(app->m_Ctx, titleX, titleY, app->m_Title, nullptr);

		nvgFillColor(app->m_Ctx, { 1.0f, 1.0f, 1.0f, 1.0f });
		nvgText(app->m_Ctx, titleX + xOffset, titleY + yOffset, app->m_Title, nullptr);

		nvgTextAlign(app->m_Ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFontSize(app->m_Ctx, 16.0f);

		const GLubyte* vendor = glGetString(GL_VENDOR);
		const GLubyte* renderer = glGetString(GL_RENDERER);
		const GLubyte* version = glGetString(GL_VERSION);
		const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

		std::string string = fmt::format("--- Renderer Details ---\nVendor: {}\nRenderer: {}\nVersion: {}\nGLSL Version: {}\n--- Engine Details ---\nEntities: {} / {}\nDeltaTime: {}\nFramerate: {}", vendor, renderer, version, glslVersion, m_Entities.size(), m_Entities.capacity(), app->m_DeltaTime, static_cast<int>((1.0f / app->m_DeltaTime)));

		nvgTextBox(app->m_Ctx, 10, 10, app->m_Size.x, string.c_str(), nullptr);

		nvgEndFrame(app->m_Ctx);

		m_Timer += app->m_DeltaTime;

		while (m_Timer > 0.12f)
		{
			m_Timer -= 0.12f;

			auto entity = std::make_shared<Entity>();
			m_Entities.push_back(entity);
		}
	}

	virtual void Attach()
	{
		
	}

	virtual void Detach()
	{

	}

	std::vector<std::shared_ptr<Entity>> m_Entities;

	double m_Timer = 0.0;
};

int main(int, char**)
{
	AF::CreateLogger();

	AF_INFO("Started");

	AF::Application app;

	app.m_StateManager.SetState(std::make_shared<MenuState>());

	app.Start();

	AF_INFO("Stopped");
	return 0;
}