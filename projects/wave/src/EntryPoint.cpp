#include <GLFW/glfw3.h>
#include <glad/glad.h>


#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <memory>
#include <vector>

#include "Log.h"
#include "Application.h"

// ------------------------------------------
// ----------------- UNUSED -----------------
// ------------------------------------------

/*class Entity
{
public:
	Entity() = default;
	virtual ~Entity() = default;

	virtual void Behaviour(AF::Application& app);

	virtual void Update(AF::Application& app);

	glm::vec2 m_Position = { 0.0f, 0.0f };
	glm::vec2 m_Size = { 32.0f, 32.0f };
	glm::vec4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

void Entity::Behaviour(AF::Application& app)
{

}

void Entity::Update(AF::Application& app)
{
	Behaviour(app);

	nvgBeginPath(app.m_Ctx);
	nvgRect(app.m_Ctx, m_Position.x, m_Position.y, m_Size.x, m_Size.y);
	nvgFillColor(app.m_Ctx, *(NVGcolor*) &m_Color);
	nvgFill(app.m_Ctx);
}*/

class MenuState : public AF::State
{
public:
	virtual void Update()
	{
		auto* app = GetStateManager()->GetApplication();

		nvgBeginFrame(app->m_Ctx, app->m_ReferenceSize.x, app->m_ReferenceSize.y, 1);
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

		nvgEndFrame(app->m_Ctx);
	}

	virtual void Attach()
	{

	}

	virtual void Detach()
	{

	}
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