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
		}
		else
		{
			Entity currentChild = shc.FirstChild;

			while (currentChild.GetComponent<SceneHierarchyComponent>().NextSibling)
				currentChild = currentChild.GetComponent<SceneHierarchyComponent>().NextSibling;

			currentChild.GetComponent<SceneHierarchyComponent>().NextSibling = entity;
			entity.GetComponent<SceneHierarchyComponent>().PreviousSibling = currentChild;
		}

		entity.GetComponent<SceneHierarchyComponent>().Parent = *this;
    }

	// -----------------------------------------------------------------------------------------------------------------------------
	void Entity::RemoveChild(Entity child)
	{
		auto& shc = GetComponent<SceneHierarchyComponent>();
		Entity currentChild = shc.FirstChild;

		while (currentChild)
		{
			if (currentChild == child)
			{
				if (currentChild == shc.FirstChild)
				{
					Entity nextSibling = currentChild.GetComponent<SceneHierarchyComponent>().NextSibling;
					shc.FirstChild = nextSibling;

					if (nextSibling)
						nextSibling.GetComponent<SceneHierarchyComponent>().PreviousSibling = {};
				}
				else
				{
					Entity prev = currentChild.GetComponent<SceneHierarchyComponent>().PreviousSibling;
					Entity next = currentChild.GetComponent<SceneHierarchyComponent>().NextSibling;

					if (prev)
						prev.GetComponent<SceneHierarchyComponent>().NextSibling = next;
					if (next)
						next.GetComponent<SceneHierarchyComponent>().PreviousSibling = prev;
				}

				currentChild.GetComponent<SceneHierarchyComponent>().NextSibling = {};
				currentChild.GetComponent<SceneHierarchyComponent>().PreviousSibling = {};
				currentChild.GetComponent<SceneHierarchyComponent>().Parent = {};

				return;
			}

			currentChild = currentChild.GetComponent<SceneHierarchyComponent>().NextSibling;
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	bool Entity::IsDescendantOf(Entity entity)
	{
		Queue<Entity> q;

		// Enqueue all the children of the queried entity
		Entity currentChild = entity.GetComponent<SceneHierarchyComponent>().FirstChild;
		while (currentChild)
		{
			q.push(currentChild);
			currentChild = currentChild.GetComponent<SceneHierarchyComponent>().NextSibling;
		}

		while (!q.empty())
		{
			Entity child = q.front();
			q.pop();

			if (child == *this)
				return true;

			currentChild = child.GetComponent<SceneHierarchyComponent>().FirstChild;
			while (currentChild)
			{
				q.push(currentChild);
				currentChild = currentChild.GetComponent<SceneHierarchyComponent>().NextSibling;
			}
		}

		return false;
	}
}
