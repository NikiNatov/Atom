#pragma once

#include "Atom/Core/Core.h"

#include <entt/entt.hpp>

namespace Atom
{
	class Scene;

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entity, Scene* scene);
		Entity(const Entity& other) = default;

		void AddChild(Entity entity);
		void RemoveChild(Entity child);
		bool IsDescendantOf(Entity entity);

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			ATOM_ENGINE_ASSERT(!HasComponent<T>(), "Component already exists!");
			T& component = m_Scene->m_Registry.emplace<T>(m_Entity, std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		void RemoveComponent()
		{
			ATOM_ENGINE_ASSERT(HasComponent<T>(), "Component does not exist!");
			m_Scene->m_Registry.remove<T>(m_Entity);
		}

		template<typename T>
		T& GetComponent()
		{
			ATOM_ENGINE_ASSERT(HasComponent<T>(), "Component does not exist!");
			return m_Scene->m_Registry.get<T>(m_Entity);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.try_get<T>(m_Entity);
		}

		inline operator bool() const { return m_Entity != entt::null; }
		inline operator uint32_t() const { return (uint32_t)m_Entity; }
		inline operator entt::entity() const { return m_Entity; }
		inline bool operator==(const Entity& other) const { return m_Entity == other.m_Entity && m_Scene == other.m_Scene; }
		inline bool operator!= (const Entity& other) const { return !(*this == other); }
	private:
		entt::entity m_Entity { entt::null };
		Scene*		 m_Scene = nullptr;
	};
}