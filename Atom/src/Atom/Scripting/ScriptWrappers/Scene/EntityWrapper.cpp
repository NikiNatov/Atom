#include "atompch.h"
#include "EntityWrapper.h"

#include "Atom/Scripting/ScriptWrappers/Scene/ComponentWrapper.h"

#include <typeinfo>

namespace Atom
{
    namespace ScriptWrappers
    {
        static HashMap<u64, std::function<bool(Atom::Entity)>> s_HasComponentFunctions = {

            { typeid(ScriptWrappers::TransformComponent).hash_code(),        [](Atom::Entity e) { return e.HasComponent<Atom::TransformComponent>(); } },
            { typeid(ScriptWrappers::CameraComponent).hash_code(),           [](Atom::Entity e) { return e.HasComponent<Atom::CameraComponent>(); } },
            { typeid(ScriptWrappers::MeshComponent).hash_code(),             [](Atom::Entity e) { return e.HasComponent<Atom::MeshComponent>(); } },
            { typeid(ScriptWrappers::SkyLightComponent).hash_code(),         [](Atom::Entity e) { return e.HasComponent<Atom::SkyLightComponent>(); } },
            { typeid(ScriptWrappers::DirectionalLightComponent).hash_code(), [](Atom::Entity e) { return e.HasComponent<Atom::DirectionalLightComponent>(); } },
            { typeid(ScriptWrappers::PointLightComponent).hash_code(),       [](Atom::Entity e) { return e.HasComponent<Atom::PointLightComponent>(); } },
            { typeid(ScriptWrappers::SpotLightComponent).hash_code(),        [](Atom::Entity e) { return e.HasComponent<Atom::SpotLightComponent>(); } },
            { typeid(ScriptWrappers::RigidbodyComponent).hash_code(),        [](Atom::Entity e) { return e.HasComponent<Atom::RigidbodyComponent>(); } },
            { typeid(ScriptWrappers::BoxColliderComponent).hash_code(),      [](Atom::Entity e) { return e.HasComponent<Atom::BoxColliderComponent>(); } },
        };

        static HashMap<u64, std::function<void(Atom::Entity)>> s_AddComponentFunctions = {

            { typeid(ScriptWrappers::TransformComponent).hash_code(),        [](Atom::Entity e) { e.AddComponent<Atom::TransformComponent>(); } },
            { typeid(ScriptWrappers::CameraComponent).hash_code(),           [](Atom::Entity e) { e.AddComponent<Atom::CameraComponent>(); } },
            { typeid(ScriptWrappers::MeshComponent).hash_code(),             [](Atom::Entity e) { e.AddComponent<Atom::MeshComponent>(); } },
            { typeid(ScriptWrappers::SkyLightComponent).hash_code(),         [](Atom::Entity e) { e.AddComponent<Atom::SkyLightComponent>(); } },
            { typeid(ScriptWrappers::DirectionalLightComponent).hash_code(), [](Atom::Entity e) { e.AddComponent<Atom::DirectionalLightComponent>(); } },
            { typeid(ScriptWrappers::PointLightComponent).hash_code(),       [](Atom::Entity e) { e.AddComponent<Atom::PointLightComponent>(); } },
            { typeid(ScriptWrappers::SpotLightComponent).hash_code(),        [](Atom::Entity e) { e.AddComponent<Atom::SpotLightComponent>(); } },
            { typeid(ScriptWrappers::RigidbodyComponent).hash_code(),        [](Atom::Entity e) { e.AddComponent<Atom::RigidbodyComponent>(); } },
            { typeid(ScriptWrappers::BoxColliderComponent).hash_code(),      [](Atom::Entity e) { e.AddComponent<Atom::BoxColliderComponent>(); } },

        };

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
            entity.GetComponent<Atom::TagComponent>().Tag = tag;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Entity::SetTranslation(const glm::vec3& translation)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            entity.GetComponent<Atom::TransformComponent>().Translation = translation;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Entity::SetRotation(const glm::vec3& rotation)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            entity.GetComponent<Atom::TransformComponent>().Rotation = rotation;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Entity::SetScale(const glm::vec3& scale)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            entity.GetComponent<Atom::TransformComponent>().Scale = scale;
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
            return entity.GetComponent<Atom::TagComponent>().Tag;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec3& Entity::GetTranslation() const
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            return entity.GetComponent<Atom::TransformComponent>().Translation;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec3& Entity::GetRotation() const
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            return entity.GetComponent<Atom::TransformComponent>().Rotation;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec3& Entity::GetScale() const
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            return entity.GetComponent<Atom::TransformComponent>().Scale;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        pybind11::object Entity::GetScriptInstance() const
        {
            Ref<ScriptInstance> instance = ScriptEngine::GetScriptInstance(UUID(m_UUID));
            return instance->GetPythonInstance();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        template<typename T>
        T Entity::AddComponent()
        {
            if (HasComponent<T>())
                return GetComponent<T>();

            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            s_AddComponentFunctions[typeid(T).hash_code()](entity);
            return T(*this);
        }

        template TransformComponent Entity::AddComponent<TransformComponent>();
        template CameraComponent Entity::AddComponent<CameraComponent>();
        template MeshComponent Entity::AddComponent<MeshComponent>();
        template SkyLightComponent Entity::AddComponent<SkyLightComponent>();
        template DirectionalLightComponent Entity::AddComponent<DirectionalLightComponent>();
        template PointLightComponent Entity::AddComponent<PointLightComponent>();
        template SpotLightComponent Entity::AddComponent<SpotLightComponent>();
        template RigidbodyComponent Entity::AddComponent<RigidbodyComponent>();
        template BoxColliderComponent Entity::AddComponent<BoxColliderComponent>();

        // -----------------------------------------------------------------------------------------------------------------------------
        template<typename T>
        bool Entity::HasComponent() const
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
            return s_HasComponentFunctions[typeid(T).hash_code()](entity);
        }

        template bool Entity::HasComponent<TransformComponent>() const;
        template bool Entity::HasComponent<CameraComponent>() const;
        template bool Entity::HasComponent<MeshComponent>() const;
        template bool Entity::HasComponent<SkyLightComponent>() const;
        template bool Entity::HasComponent<DirectionalLightComponent>() const;
        template bool Entity::HasComponent<PointLightComponent>() const;
        template bool Entity::HasComponent<SpotLightComponent>() const;
        template bool Entity::HasComponent<RigidbodyComponent>() const;
        template bool Entity::HasComponent<BoxColliderComponent>() const;

        // -----------------------------------------------------------------------------------------------------------------------------
        template<typename T>
        T Entity::GetComponent() const
        {
            if (!HasComponent<T>())
                return T();

            return T(*this);
        }

        template TransformComponent Entity::GetComponent<TransformComponent>() const;
        template CameraComponent Entity::GetComponent<CameraComponent>() const;
        template MeshComponent Entity::GetComponent<MeshComponent>() const;
        template SkyLightComponent Entity::GetComponent<SkyLightComponent>() const;
        template DirectionalLightComponent Entity::GetComponent<DirectionalLightComponent>() const;
        template PointLightComponent Entity::GetComponent<PointLightComponent>() const;
        template SpotLightComponent Entity::GetComponent<SpotLightComponent>() const;
        template RigidbodyComponent Entity::GetComponent<RigidbodyComponent>() const;
        template BoxColliderComponent Entity::GetComponent<BoxColliderComponent>() const;

        // -----------------------------------------------------------------------------------------------------------------------------
        Entity Entity::FindEntityByName(const String& name)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->FindEntityByName(name);
            return Entity(entity ? entity.GetUUID() : 0);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Entity Entity::CreateEntity(const String& name)
        {
            Scene* scene = ScriptEngine::GetRunningScene();
            Atom::Entity entity = scene->CreateEntity(name);
            return Entity(entity.GetUUID());
        }
    }
}