#include "atompch.h"
#include "Entity.h"

#include "Atom/Scene/Scene.h"
#include "Atom/Scene/Components.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Entity::Entity(entt::entity entity, Scene* scene)
        : m_Entity(entity), m_Scene(scene)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Entity::AddChild(Entity entity)
    {
		ATOM_ENGINE_ASSERT(entity != *this);

		auto& shc = GetComponent<SceneHierarchyComponent>();

		if (!shc.FirstChild)
		{
			shc.FirstChild = entity;
			shc.LastChild = entity;
		}
		else
		{
			shc.LastChild.GetComponent<SceneHierarchyComponent>().NextSibling = entity;
			entity.GetComponent<SceneHierarchyComponent>().PreviousSibling = shc.LastChild;
			shc.LastChild = entity;
		}

		entity.GetComponent<SceneHierarchyComponent>().Parent = *this;
    }
}
