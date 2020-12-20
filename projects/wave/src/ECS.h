#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>
#include <typeinfo>

namespace AF::ECS
{
	struct Scene;
	struct Entity;

	struct Component
	{
		Component() = default;
		virtual ~Component() = default;

		virtual void Start();

		virtual void Update();

		std::weak_ptr<Entity> m_Entity = {};
	};

	struct Entity final : public std::enable_shared_from_this<Entity>
	{
		Entity(std::weak_ptr<Scene> scene);

		template<typename t_Type, typename... t_Args>
		std::shared_ptr<t_Type> CreateComponent(t_Args&&... args)
		{
			std::shared_ptr<t_Type> component = std::make_shared<t_Type>(std::forward<t_Args>(args)...);
			component->m_Entity = weak_from_this();
			m_Components.insert(std::make_pair(typeid(t_Type).hash_code(), component));
			return component;
		}

		template<typename t_Type>
		std::shared_ptr<t_Type> GetComponent()
		{
			size_t id = typeid(t_Type).hash_code();

			auto result = m_Components.find(id);
			if (result == m_Components.end()) return {};

			return result->second;
		}

		template<typename t_Type>
		std::shared_ptr<t_Type> DestroyComponent()
		{
			size_t id = typeid(t_Type).hash_code();

			auto result = m_Components.find(id);
			if (result == m_Components.end()) return {};

			std::shared_ptr<t_Type> component = result->second;
			component->m_Entity = {};
			m_Components.remove(id);

			return component;
		}

		void Update();

		void Kill();

		std::weak_ptr<Scene> m_Scene;
		std::unordered_map<size_t, std::shared_ptr<Component>> m_Components;
		bool m_FirstFrame = true;
	};

	struct Scene final : public std::enable_shared_from_this<Scene>
	{
		Scene() = default;
		~Scene();

		std::shared_ptr<Entity> CreateEntity();
		std::shared_ptr<Entity> DestroyEntity(std::shared_ptr<Entity> entity);

		void Clear();
		void Update();

		std::vector<std::shared_ptr<Entity>> m_Entities;
	};
}