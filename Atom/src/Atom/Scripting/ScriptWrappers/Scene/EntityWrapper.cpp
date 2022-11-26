#include "atompch.h"
#include "EntityWrapper.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        // -----------------------------------------------------------------------------------------------------------------------------
        Entity::Entity()
            : m_UUID(0)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Entity::Entity(u64 uuid)
            : m_UUID(uuid)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        u64 Entity::GetUUID() const
        {
            return m_UUID;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const String& Entity::GetTag() const
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            return entity.GetComponent<TagComponent>().Tag;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Entity Entity::FindEntityByName(const String& name)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByName(name);
            return Entity(entity ? entity.GetUUID() : 0);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        pybind11::object Entity::GetScriptInstance() const
        {
            Ref<ScriptInstance> instance = ScriptEngine::GetScriptInstance(UUID(m_UUID));
            return instance->GetPythonInstance();
        }
    }
}