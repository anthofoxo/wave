#include <GLFW/glfw3.h>
#include <glad/glad.h>


#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <memory>
#include <vector>

#include "Log.h"
#include "Application.h"

#if defined(AF_CONF_DEBUG)
	#define AF_CONF_STR "DEBUG"
#elif defined(AF_CONF_RELEASE)
	#define AF_CONF_STR "RELEASE"
#elif defined(AF_CONF_DIST)
	#define AF_CONF_STR "DIST"
#else
	#error "Invalid configuration"
#endif

#define AF_MAIN() int main(int, char**)
#if defined(AF_PLAT_WINDOWS) && defined(AF_CONF_DIST)
	#undef AF_MAIN
	#define AF_MAIN() int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#endif

struct EntityManager;

class Entity
{
public:
	Entity() = default;

	virtual ~Entity() = default;

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
		m_Entities.push_back(entity);
	}

	void RemoveEntity(std::shared_ptr<Entity> entity)
	{
		entity->m_Manager = nullptr;
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
		{
			m_Entities.erase(std::remove(m_Entities.begin(), m_Entities.end(), entity));
		}
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

	nvgBeginPath(app->m_Ctx);
	nvgRect(app->m_Ctx, m_Position.x, m_Position.y, m_Size.x, m_Size.y);
	nvgFillColor(app->m_Ctx, *(NVGcolor*) &m_Color);
	nvgFill(app->m_Ctx);
}


class Trail : public Entity
{
public:
	Trail(float lifetime, glm::vec2 position, glm::vec4 color)
		: m_MaxLifetime(lifetime), m_CurrentLifetime(lifetime)
	{
		m_Position = position;
		m_Color = color;
	}

	virtual ~Trail() = default;

	virtual void Behaviour() override
	{
		m_CurrentLifetime -= static_cast<float>(m_Manager->m_State->GetStateManager()->GetApplication()->m_DeltaTime);

		if (m_CurrentLifetime <= 0)
		{
			m_KillMe = true;
			m_CurrentLifetime = 0.0f;
		}

		m_Color.a = m_CurrentLifetime / m_MaxLifetime;
	}

	float m_MaxLifetime;
	float m_CurrentLifetime;
};


class MenuParticle : public Entity
{
public:
	MenuParticle()
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

	virtual ~MenuParticle() = default;

	virtual void Behaviour() override
	{
		m_Color.r = glm::linearRand<float>(0.0f, 1.0f);
		m_Color.g = glm::linearRand<float>(0.0f, 1.0f);
		m_Color.b = glm::linearRand<float>(0.0f, 1.0f);

		if (m_Position.x > 1280.0f + m_Size.x * 2.0f) m_KillMe = true;
		if (m_Position.y > 720.0f + m_Size.y * 2.0f) m_KillMe = true;

		if (m_Position.x < -m_Size.x * 2.0f) m_KillMe = true;
		if (m_Position.y < -m_Size.y * 2.0f) m_KillMe = true;

		m_Timer += static_cast<float>(m_Manager->m_State->GetStateManager()->GetApplication()->m_DeltaTime);

		if (m_Timer > 0.05f)
		{
			m_Timer -= 0.05f;

			auto trail = std::make_shared<Trail>(0.3f, m_Position, m_Color);
			m_Manager->AddEntity(trail);
		}

		
	}

	float m_Timer = 0.0f;
};

class MenuState : public AF::State
{
public:
	virtual void Update()
	{
		auto* app = GetStateManager()->GetApplication();

		nvgBeginFrame(app->m_Ctx, app->m_ReferenceSize.x, app->m_ReferenceSize.y, 1);

		m_EntityManager.Update();

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
		nvgFontSize(app->m_Ctx, 12.0f);

		const GLubyte* vendor = glGetString(GL_VENDOR);
		const GLubyte* renderer = glGetString(GL_RENDERER);
		const GLubyte* version = glGetString(GL_VERSION);
		const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

		std::string string = fmt::format("{} {} {}\n--- Renderer Details ---\nVendor: {}\nRenderer: {}\nVersion: {}\nGLSL Version: {}\n--- Engine Details ---\nEntities: {} / {}\nDeltaTime: {}\nFramerate: {}", AF_CONF_STR, __DATE__, __TIME__, vendor, renderer, version, glslVersion, m_EntityManager.GetSize(), m_EntityManager.GetCapacity(), app->m_DeltaTime, static_cast<int>((1.0f / app->m_DeltaTime)));

		constexpr float margin = 8.0f;

		nvgTextBox(app->m_Ctx, margin, margin, app->m_Size.x - margin * 2.0f, string.c_str(), nullptr);

		nvgEndFrame(app->m_Ctx);

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

AF_MAIN()
{
	AF::CreateLogger();

	AF_INFO("Started");

	AF::Application app;

	app.m_StateManager.SetState(std::make_shared<MenuState>());

	app.Start();

	AF_INFO("Stopped");
	return 0;
}