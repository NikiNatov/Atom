#include "atompch.h"
#include "Entity.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Entity::Entity(entt::entity entity, Scene* scene)
        : m_Entity(entity), m_Scene(scene)
    {
    }
}
