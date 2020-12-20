#include "ECS.h"

namespace AF::ECS
{
	void Component::Start()
	{
	}

	void Component::Update()
	{
	}

	Entity::Entity(std::weak_ptr<Scene> scene)
	{
		m_Scene = scene;
	}

	void Entity::Update()
	{
		if (m_FirstFrame)
		{
			for (auto [key, value] : m_Components)
				value->Start();

			m_FirstFrame = false;
		}

		for (auto [key, value] : m_Components)
			value->Update();
	}

	void Entity::Kill()
	{
		if (std::shared_ptr<Scene> scene = m_Scene.lock())
			scene->DestroyEntity(shared_from_this());
	}

	Scene::~Scene()
	{
		Clear();
	}

	std::shared_ptr<Entity> Scene::CreateEntity()
	{
		std::shared_ptr<Entity> entity = std::make_shared<Entity>(weak_from_this());
		m_Entities.push_back(entity);
		return entity;
	}

	std::shared_ptr<Entity> Scene::DestroyEntity(std::shared_ptr<Entity> entity)
	{
		m_Entities.erase(std::remove(m_Entities.begin(), m_Entities.end(), entity));
		entity->m_Scene = {};

		return entity;
	}

	void Scene::Clear()
	{
		for (auto entity : m_Entities)
			entity->m_Scene = {};

		m_Entities.clear();
	}

	void Scene::Update()
	{
		for (auto entity : m_Entities)
			entity->Update();
	}
}