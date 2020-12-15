#include "ecs.h"

#include <functional>

namespace ECS
{
	size_t TypeInfo(const std::string& str)
	{
		return std::hash<std::string>{}(str);
	}

	void Component::OnUpdate()
	{
	}

	void Component::OnGui()
	{
	}

	std::unordered_map<size_t, ComponentFunctionPointers> ComponentFunctionPointers::s_ComponentFunctions;

	Entity::Entity(Scene* scene)
		: m_Scene(scene)
	{
	}

	void Entity::OnUpdate()
	{
		for (auto& [id, component] : m_Components)
			component->OnUpdate();
	}

	bool Entity::IsRoot()
	{
		return m_Parent == nullptr;
	}


	void Entity::SetParent(std::shared_ptr<Entity> parent)
	{
		if (m_Parent && !parent)
		{
			auto& children = m_Parent->m_Children;
			children.erase(std::remove(children.begin(), children.end(), shared_from_this()));
			m_Parent = parent;
		}

		if (!m_Parent && parent)
		{
			m_Parent = parent;
			m_Parent->m_Children.push_back(shared_from_this());
		}
	}

	std::shared_ptr<Component> Entity::CreateComponent(std::string id)
	{
		return CreateComponent(TypeInfo(id));
	}

	std::shared_ptr<Component> Entity::CreateComponent(size_t id)
	{
		if (HasComponent(id))
			return nullptr;

		std::shared_ptr<Component> component = ComponentFunctionPointers::s_ComponentFunctions[id].Create();
		component->m_Entity = shared_from_this();

		m_Components[id] = component;
		return component;
	}

	std::shared_ptr<Component> Entity::AddComponent(std::string id, std::shared_ptr<Component> component)
	{
		return AddComponent(TypeInfo(id), component);
	}

	std::shared_ptr<Component> Entity::AddComponent(size_t id, std::shared_ptr<Component> component)
	{
		if (!component) return nullptr;
		if (HasComponent(id)) return nullptr;
		if (component->m_Entity) return nullptr;

		component->m_Entity = shared_from_this();
		m_Components[id] = component;

		return component;
	}

	bool Entity::HasComponent(std::string id)
	{
		return HasComponent(TypeInfo(id));
	}

	bool Entity::HasComponent(size_t id)
	{
		return m_Components.find(id) != m_Components.end();
	}

	std::shared_ptr<Component> Entity::GetComponent(std::string id)
	{
		return GetComponent(TypeInfo(id));
	}

	std::shared_ptr<Component> Entity::GetComponent(size_t id)
	{
		auto result = m_Components.find(id);

		if (result == m_Components.end()) return nullptr;
		return result->second;
	}

	std::shared_ptr<Component> Entity::RemoveComponent(std::string id)
	{
		return RemoveComponent(TypeInfo(id));
	}

	std::shared_ptr<Component> Entity::RemoveComponent(size_t id)
	{
		auto result = m_Components.find(id);

		if (result == m_Components.end()) return nullptr;

		std::shared_ptr<Component> ref = result->second;
		ref->m_Entity = nullptr;

		m_Components.erase(id);

		return ref;
	}

	Scene::~Scene()
	{
		Clear();
	}

	void Scene::OnUpdate()
	{
		for (std::shared_ptr<Entity> entity : m_Entities)
			entity->OnUpdate();
	}

	void Scene::Clear()
	{
		for (std::shared_ptr<Entity> entity : m_Entities)
			entity->m_Scene = nullptr;

		m_Entities.clear();
	}

	std::shared_ptr<Entity> Scene::CreateEntity()
	{
		std::shared_ptr<Entity> entity = std::make_shared<Entity>(this);
		m_Entities.push_back(entity);
		return entity;
	}

	std::shared_ptr<Entity> Scene::AddEntity(std::shared_ptr<Entity> entity)
	{
		if (entity->m_Scene)
			return nullptr;

		entity->m_Scene = this;
		m_Entities.push_back(entity);

		return entity;
	}

	bool Scene::HasEntity(std::shared_ptr<Entity> entity)
	{
		return std::find(m_Entities.begin(), m_Entities.end(), entity) != m_Entities.end();
	}

	std::shared_ptr<Entity> Scene::RemoveEntity(std::shared_ptr<Entity> entity)
	{
		auto result = std::find(m_Entities.begin(), m_Entities.end(), entity);

		if (result == m_Entities.end()) return nullptr;

		m_Entities.erase(result);

		entity->m_Scene = nullptr;

		return entity;
	}
}