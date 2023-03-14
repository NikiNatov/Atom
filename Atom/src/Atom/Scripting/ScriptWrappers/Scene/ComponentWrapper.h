#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/UUID.h"
#include "Atom/Scene/Components.h"
#include "Atom/Scene/Scene.h"
#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Scripting/ScriptWrappers/Scene/EntityWrapper.h"
#include "Atom/Scripting/ScriptWrappers/Renderer/MeshWrapper.h"
#include "Atom/Scripting/ScriptWrappers/Renderer/TextureWrapper.h"
#include "Atom/Scripting/ScriptWrappers/Renderer/AnimationWrapper.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
            glm::vec3 GetUpVector();
            glm::vec3 GetRightVector();
            glm::vec3 GetForwardVector();
        private:
            glm::quat GetOrientation();
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

        class MeshComponent : public Component
        {
        public:
            MeshComponent() = default;
            MeshComponent(Entity entity);

            void SetMesh(Mesh mesh);
            Mesh GetMesh();
        };

        class AnimatedMeshComponent : public Component
        {
        public:
            AnimatedMeshComponent() = default;
            AnimatedMeshComponent(Entity entity);

            void SetMesh(Mesh mesh);
            Mesh GetMesh();
        };

        class AnimatorComponent : public Component
        {
        public:
            AnimatorComponent() = default;
            AnimatorComponent(Entity entity);

            void SetAnimationController(AnimationController controller);
            void SetTime(f32 time);
            void SetPlay(bool play);
            AnimationController GetAnimationController();
            f32 GetTime();
            bool GetPlay();
        };

        class SkyLightComponent : public Component
        {
        public:
            SkyLightComponent() = default;
            SkyLightComponent(Entity entity);

            void SetEnvironmentMap(TextureCube environmentMap);
            TextureCube GetEnvironmentMap();
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
            void SetVelocity(const glm::vec3& velocity);
            Atom::RigidbodyComponent::RigidbodyType GetType();
            f32 GetMass();
            const glm::bvec3& GetFixedRotation();
            glm::vec3 GetVelocity();

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

        class SphereColliderComponent : public Component
        {
        public:
            SphereColliderComponent() = default;
            SphereColliderComponent(Entity entity);

            void SetCenter(const glm::vec3& center);
            void SetRadius(f32 radius);
            void SetRestitution(f32 restitution);
            void SetStaticFriction(f32 staticFriction);
            void SetDynamicFriction(f32 dynamicFriction);
            const glm::vec3& GetCenter();
            f32 GetRadius();
            f32 GetRestitution();
            f32 GetStaticFriction();
            f32 GetDynamicFriction();
        };
    }
}