#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/UUID.h"
#include "Atom/Scene/Components.h"
#include "Atom/Scene/Scene.h"
#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Scripting/ScriptWrappers/Scene/EntityWrapper.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        class Component
        {
        public:
            Component() = default;
            Component(Entity entity);
            virtual ~Component() = default;

            inline bool IsValid() { return m_Entity.IsValid(); }
            inline Entity GetEntity() { return m_Entity; }
        protected:
            Entity m_Entity;
        };

        class TransformComponent : public Component
        {
        public:
            TransformComponent() = default;
            TransformComponent(Entity entity);

            void SetTranslation(const glm::vec3& translation);
            void SetRotation(const glm::vec3& rotation);
            void SetScale(const glm::vec3& scale);
            const glm::vec3& GetTranslation();
            const glm::vec3& GetRotation();
            const glm::vec3& GetScale();
        };

        class CameraComponent : public Component
        {
        public:
            CameraComponent() = default;
            CameraComponent(Entity entity);

            void SetCamera(const SceneCamera& camera);
            void SetFixedAspectRatio(bool state);
            void SetIsPrimaryCamera(bool state);
            SceneCamera& GetCamera();
            bool GetFixedAspectRatio();
            bool GetIsPrimaryCamera();
        };

        class DirectionalLightComponent : public Component
        {
        public:
            DirectionalLightComponent() = default;
            DirectionalLightComponent(Entity entity);

            void SetColor(const glm::vec3& color);
            void SetIntensity(f32 intensity);
            const glm::vec3& GetColor();
            f32 GetIntensity();
        };

        class PointLightComponent : public Component
        {
        public:
            PointLightComponent() = default;
            PointLightComponent(Entity entity);

            void SetColor(const glm::vec3& color);
            void SetIntensity(f32 intensity);
            void SetAttenuation(const glm::vec3& attenuation);
            const glm::vec3& GetColor();
            f32 GetIntensity();
            const glm::vec3& GetAttenuation();
        };

        class SpotLightComponent : public Component
        {
        public:
            SpotLightComponent() = default;
            SpotLightComponent(Entity entity);

            void SetColor(const glm::vec3& color);
            void SetIntensity(f32 intensity);
            void SetDirection(const glm::vec3& direction);
            void SetConeAngle(f32 angle);
            void SetAttenuation(const glm::vec3& attenuation);
            const glm::vec3& GetColor();
            f32 GetIntensity();
            const glm::vec3& GetDirection();
            f32 GetConeAngle();
            const glm::vec3& GetAttenuation();
        };

        class RigidbodyComponent : public Component
        {
        public:
            RigidbodyComponent() = default;
            RigidbodyComponent(Entity entity);

            void SetType(Atom::RigidbodyComponent::RigidbodyType type);
            void SetMass(f32 mass);
            void SetFixedRotation(const glm::bvec3& fixedRotation);
            Atom::RigidbodyComponent::RigidbodyType GetType();
            f32 GetMass();
            const glm::bvec3& GetFixedRotation();

            void AddForce(const glm::vec3& force, bool awake = true);
            void AddImpulse(const glm::vec3& impulse, bool awake = true);
        };

        class BoxColliderComponent : public Component
        {
        public:
            BoxColliderComponent() = default;
            BoxColliderComponent(Entity entity);

            void SetCenter(const glm::vec3& center);
            void SetSize(const glm::vec3& size);
            void SetRestitution(f32 restitution);
            void SetStaticFriction(f32 staticFriction);
            void SetDynamicFriction(f32 dynamicFriction);
            const glm::vec3& GetCenter();
            const glm::vec3& GetSize();
            f32 GetRestitution();
            f32 GetStaticFriction();
            f32 GetDynamicFriction();
        };
    }
}