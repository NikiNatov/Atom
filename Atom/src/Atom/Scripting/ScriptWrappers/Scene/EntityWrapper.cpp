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
        void Entity::SetTag(const String& tag)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            entity.GetComponent<TagComponent>().Tag = tag;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Entity::SetTranslation(const glm::vec3& translation)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            entity.GetComponent<TransformComponent>().Translation = translation;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Entity::SetRotation(const glm::vec3& rotation)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            entity.GetComponent<TransformComponent>().Rotation = rotation;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Entity::SetScale(const glm::vec3& scale)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            entity.GetComponent<TransformComponent>().Scale = scale;
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
        glm::vec3& Entity::GetTranslation() const
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            return entity.GetComponent<TransformComponent>().Translation;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec3& Entity::GetRotation() const
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            return entity.GetComponent<TransformComponent>().Rotation;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec3& Entity::GetScale() const
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            return entity.GetComponent<TransformComponent>().Scale;
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