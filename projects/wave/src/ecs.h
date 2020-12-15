#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#define ECS_INTERNAL_DECLARE()\
virtual std::string ecsVName() = 0;\
virtual size_t ecsVId() = 0;

namespace ECS
{
	struct Scene;
	struct Entity;

	size_t TypeInfo(const std::string& str);

	struct Component
	{
		Component() = default;
		virtual ~Component() = default;

		virtual void OnUpdate();
		virtual void OnGui();

		std::shared_ptr<Entity> m_Entity = nullptr;

		ECS_INTERNAL_DECLARE();
	};

	struct ComponentFunctionPointers
	{
		std::shared_ptr<Component>(*Create)();

		static std::unordered_map<size_t, ComponentFunctionPointers> s_ComponentFunctions;
	};

	struct Entity final : std::enable_shared_from_this<Entity>
	{
		Entity(Scene* scene);

		void OnUpdate();

		bool IsRoot();

		void SetParent(std::shared_ptr<Entity> parent);

		template<typename T>
		std::shared_ptr<T> CreateComponent()
		{
			return std::dynamic_pointer_cast<T>(CreateComponent(T::ecsName()));
		}

		std::shared_ptr<Component> CreateComponent(std::string id);
		std::shared_ptr<Component> CreateComponent(size_t id);

		template<typename T>
		std::shared_ptr<T> AddComponent(std::shared_ptr<T> component)
		{
			return std::dynamic_pointer_cast<T>(AddComponent(component->ecsVName(), component));
		}

		std::shared_ptr<Component> AddComponent(std::string id, std::shared_ptr<Component> component);
		std::shared_ptr<Component> AddComponent(size_t id, std::shared_ptr<Component> component);

		template<typename T>
		bool HasComponent()
		{
			return HasComponent(T::ecsName());
		}

		bool HasComponent(std::string id);
		bool HasComponent(size_t id);

		template<typename T>
		std::shared_ptr<T> GetComponent()
		{
			return std::dynamic_pointer_cast<T>(GetComponent(T::ecsName()));
		}

		std::shared_ptr<Component> GetComponent(std::string id);
		std::shared_ptr<Component> GetComponent(size_t id);

		template<typename T>
		std::shared_ptr<T> RemoveComponent()
		{
			return dynamic_pointer_cast<T>(RemoveComponent(T::ecsName()));
		}

		std::shared_ptr<Component> RemoveComponent(std::string id);
		std::shared_ptr<Component> RemoveComponent(size_t id);

		Scene* m_Scene = nullptr;

		std::unordered_map<size_t, std::shared_ptr<Component>> m_Components;

		std::shared_ptr<Entity> m_Parent = nullptr;
		std::vector<std::shared_ptr<Entity>> m_Children;
	};

	struct Scene final
	{
		Scene() = default;
		~Scene();

		void OnUpdate();

		void Clear();

		std::shared_ptr<Entity> CreateEntity();
		std::shared_ptr<Entity> AddEntity(std::shared_ptr<Entity> entity);
		bool HasEntity(std::shared_ptr<Entity> entity);
		std::shared_ptr<Entity> RemoveEntity(std::shared_ptr<Entity> entity);

		std::vector<std::shared_ptr<Entity>> m_Entities;
	};
}

#define ECS_DECLARE()\
static std::shared_ptr<::ECS::Component> ecsCreate();\
static std::string ecsName();\
static size_t ecsId();\
virtual std::string ecsVName();\
virtual size_t ecsVId();

#define ECS_DEFINE(T)\
std::shared_ptr<::ECS::Component> T::ecsCreate() { return std::make_shared<T>(); }\
std::string T::ecsName() { return #T; }\
size_t T::ecsId() { return ::ECS::TypeInfo(#T); }\
std::string T::ecsVName() { return #T; }\
size_t T::ecsVId() { return ::ECS::TypeInfo(#T); }

#define ECS_LOAD(T)\
::ECS::ComponentFunctionPointers::s_ComponentFunctions[::ECS::TypeInfo(#T)] = { &T::ecsCreate }