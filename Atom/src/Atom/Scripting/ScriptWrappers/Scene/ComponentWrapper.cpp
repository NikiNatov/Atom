#include "atompch.h"
#include "ComponentWrapper.h"

#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Physics/PhysicsEngine.h"
#include "Atom/Asset/AssetManager.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

namespace Atom::ScriptWrappers
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Component::Component(Entity entity)
        : m_Entity(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TransformComponent::TransformComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TransformComponent::SetTranslation(const glm::vec3& translation)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::TransformComponent>().Translation = translation;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TransformComponent::SetRotation(const glm::vec3& rotation)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::TransformComponent>().Rotation = rotation;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TransformComponent::SetScale(const glm::vec3& scale)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::TransformComponent>().Scale = scale;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& TransformComponent::GetTranslation()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::TransformComponent>().Translation;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& TransformComponent::GetRotation()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::TransformComponent>().Rotation;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& TransformComponent::GetScale()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::TransformComponent>().Scale;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::vec3 TransformComponent::GetUpVector()
    {
        return glm::normalize(GetOrientation() * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::vec3 TransformComponent::GetRightVector()
    {
        return glm::normalize(GetOrientation() * glm::vec3(1.0f, 0.0f, 0.0f));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::vec3 TransformComponent::GetForwardVector()
    {
        return glm::normalize(GetOrientation() * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::quat TransformComponent::GetOrientation()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        auto& tc = entity.GetComponent<Atom::TransformComponent>();

        return glm::normalize(glm::angleAxis(tc.Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
                              glm::angleAxis(tc.Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RigidbodyComponent::RigidbodyComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RigidbodyComponent::SetType(Atom::RigidbodyComponent::RigidbodyType type)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::RigidbodyComponent>().Type = type;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RigidbodyComponent::SetMass(f32 mass)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::RigidbodyComponent>().Mass = mass;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RigidbodyComponent::SetFixedRotation(const glm::bvec3& fixedRotation)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::RigidbodyComponent>().FixedRotation = fixedRotation;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RigidbodyComponent::SetVelocity(const glm::vec3& velocity)
    {
        if (physx::PxRigidActor* actor = PhysicsEngine::GetRigidBody(m_Entity.GetUUID()))
        {
            if (physx::PxRigidDynamic* rb = actor->is<physx::PxRigidDynamic>())
            {
                rb->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Atom::RigidbodyComponent::RigidbodyType RigidbodyComponent::GetType()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::RigidbodyComponent>().Type;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 RigidbodyComponent::GetMass()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::RigidbodyComponent>().Mass;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::bvec3& RigidbodyComponent::GetFixedRotation()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::RigidbodyComponent>().FixedRotation;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::vec3 RigidbodyComponent::GetVelocity()
    {
        if (physx::PxRigidActor* actor = PhysicsEngine::GetRigidBody(m_Entity.GetUUID()))
        {
            if (physx::PxRigidDynamic* rb = actor->is<physx::PxRigidDynamic>())
            {
                physx::PxVec3 velocity = rb->getLinearVelocity();
                return { velocity.x, velocity.y, velocity.z };
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RigidbodyComponent::AddForce(const glm::vec3& force, bool awake)
    {
        if (physx::PxRigidActor* actor = PhysicsEngine::GetRigidBody(m_Entity.GetUUID()))
        {
            if (physx::PxRigidDynamic* rb = actor->is<physx::PxRigidDynamic>())
            {
                rb->addForce(physx::PxVec3(force.x, force.y, force.z), physx::PxForceMode::eFORCE, awake);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RigidbodyComponent::AddImpulse(const glm::vec3& impulse, bool awake)
    {
        if (physx::PxRigidActor* actor = PhysicsEngine::GetRigidBody(m_Entity.GetUUID()))
        {
            if (physx::PxRigidDynamic* rb = actor->is<physx::PxRigidDynamic>())
            {
                rb->addForce(physx::PxVec3(impulse.x, impulse.y, impulse.z), physx::PxForceMode::eIMPULSE, awake);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CameraComponent::CameraComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CameraComponent::SetCamera(const SceneCamera& camera)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::CameraComponent>().Camera = camera;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CameraComponent::SetFixedAspectRatio(bool state)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::CameraComponent>().FixedAspectRatio = state;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CameraComponent::SetIsPrimaryCamera(bool state)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::CameraComponent>().Primary = state;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SceneCamera& CameraComponent::GetCamera()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::CameraComponent>().Camera;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool CameraComponent::GetFixedAspectRatio()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::CameraComponent>().FixedAspectRatio;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool CameraComponent::GetIsPrimaryCamera()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::CameraComponent>().Primary;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    MeshComponent::MeshComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void MeshComponent::SetMesh(Mesh mesh)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::MeshComponent>().Mesh = mesh.GetMesh();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh MeshComponent::GetMesh()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return Mesh(entity.GetComponent<Atom::MeshComponent>().Mesh);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    AnimatedMeshComponent::AnimatedMeshComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AnimatedMeshComponent::SetMesh(Mesh mesh)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::AnimatedMeshComponent>().Mesh = mesh.GetMesh();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh AnimatedMeshComponent::GetMesh()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return Mesh(entity.GetComponent<Atom::AnimatedMeshComponent>().Mesh);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    AnimatorComponent::AnimatorComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AnimatorComponent::SetAnimationController(AnimationController controller)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::AnimatorComponent>().AnimationController = controller.GetAnimationController();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AnimatorComponent::SetTime(f32 time)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::AnimatorComponent>().CurrentTime = time;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AnimatorComponent::SetPlay(bool play)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::AnimatorComponent>().Play = play;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    AnimationController AnimatorComponent::GetAnimationController()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return AnimationController(entity.GetComponent<Atom::AnimatorComponent>().AnimationController);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 AnimatorComponent::GetTime()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::AnimatorComponent>().CurrentTime;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool AnimatorComponent::GetPlay()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::AnimatorComponent>().Play;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SkyLightComponent::SkyLightComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SkyLightComponent::SetEnvironmentMap(TextureCube environmentMap)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SkyLightComponent>().EnvironmentMap = environmentMap.GetTexture();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureCube SkyLightComponent::GetEnvironmentMap()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return TextureCube(entity.GetComponent<Atom::SkyLightComponent>().EnvironmentMap);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DirectionalLightComponent::DirectionalLightComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DirectionalLightComponent::SetColor(const glm::vec3& color)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::DirectionalLightComponent>().Color = color;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DirectionalLightComponent::SetIntensity(f32 intensity)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::DirectionalLightComponent>().Intensity = intensity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& DirectionalLightComponent::GetColor()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::DirectionalLightComponent>().Color;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 DirectionalLightComponent::GetIntensity()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::DirectionalLightComponent>().Intensity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    PointLightComponent::PointLightComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PointLightComponent::SetColor(const glm::vec3& color)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::PointLightComponent>().Color = color;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PointLightComponent::SetIntensity(f32 intensity)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::PointLightComponent>().Intensity = intensity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PointLightComponent::SetAttenuation(const glm::vec3& attenuation)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::PointLightComponent>().AttenuationFactors = attenuation;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& PointLightComponent::GetColor()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::PointLightComponent>().Color;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 PointLightComponent::GetIntensity()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::PointLightComponent>().Intensity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& PointLightComponent::GetAttenuation()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::PointLightComponent>().AttenuationFactors;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SpotLightComponent::SpotLightComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SpotLightComponent::SetColor(const glm::vec3& color)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SpotLightComponent>().Color = color;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SpotLightComponent::SetIntensity(f32 intensity)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SpotLightComponent>().Intensity = intensity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SpotLightComponent::SetDirection(const glm::vec3& direction)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SpotLightComponent>().Direction = direction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SpotLightComponent::SetConeAngle(f32 angle)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SpotLightComponent>().ConeAngle = angle;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SpotLightComponent::SetAttenuation(const glm::vec3& attenuation)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SpotLightComponent>().AttenuationFactors = attenuation;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& SpotLightComponent::GetColor()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SpotLightComponent>().Color;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 SpotLightComponent::GetIntensity()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SpotLightComponent>().Intensity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& SpotLightComponent::GetDirection()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SpotLightComponent>().Direction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 SpotLightComponent::GetConeAngle()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SpotLightComponent>().ConeAngle;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& SpotLightComponent::GetAttenuation()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SpotLightComponent>().AttenuationFactors;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    BoxColliderComponent::BoxColliderComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BoxColliderComponent::SetCenter(const glm::vec3& center)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::BoxColliderComponent>().Center = center;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BoxColliderComponent::SetSize(const glm::vec3& size)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::BoxColliderComponent>().Size = size;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BoxColliderComponent::SetRestitution(f32 restitution)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::BoxColliderComponent>().Restitution = restitution;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BoxColliderComponent::SetStaticFriction(f32 staticFriction)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::BoxColliderComponent>().StaticFriction = staticFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void BoxColliderComponent::SetDynamicFriction(f32 dynamicFriction)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::BoxColliderComponent>().DynamicFriction = dynamicFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& BoxColliderComponent::GetCenter()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::BoxColliderComponent>().Center;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& BoxColliderComponent::GetSize()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::BoxColliderComponent>().Size;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 BoxColliderComponent::GetRestitution()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::BoxColliderComponent>().Restitution;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 BoxColliderComponent::GetStaticFriction()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::BoxColliderComponent>().StaticFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 BoxColliderComponent::GetDynamicFriction()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::BoxColliderComponent>().DynamicFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SphereColliderComponent::SphereColliderComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SphereColliderComponent::SetCenter(const glm::vec3& center)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SphereColliderComponent>().Center = center;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SphereColliderComponent::SetRadius(f32 radius)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SphereColliderComponent>().Radius = radius;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SphereColliderComponent::SetRestitution(f32 restitution)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SphereColliderComponent>().Restitution = restitution;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SphereColliderComponent::SetStaticFriction(f32 staticFriction)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SphereColliderComponent>().StaticFriction = staticFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SphereColliderComponent::SetDynamicFriction(f32 dynamicFriction)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::SphereColliderComponent>().DynamicFriction = dynamicFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& SphereColliderComponent::GetCenter()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SphereColliderComponent>().Center;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 SphereColliderComponent::GetRadius()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SphereColliderComponent>().Radius;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 SphereColliderComponent::GetRestitution()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SphereColliderComponent>().Restitution;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 SphereColliderComponent::GetStaticFriction()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SphereColliderComponent>().StaticFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 SphereColliderComponent::GetDynamicFriction()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::SphereColliderComponent>().DynamicFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CapsuleColliderComponent::CapsuleColliderComponent(Entity entity)
        : Component(entity)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CapsuleColliderComponent::SetCenter(const glm::vec3& center)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::CapsuleColliderComponent>().Center = center;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CapsuleColliderComponent::SetRadius(f32 radius)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::CapsuleColliderComponent>().Radius = radius;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CapsuleColliderComponent::SetHeight(f32 height)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::CapsuleColliderComponent>().Height = height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CapsuleColliderComponent::SetRestitution(f32 restitution)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::CapsuleColliderComponent>().Restitution = restitution;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CapsuleColliderComponent::SetStaticFriction(f32 staticFriction)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::CapsuleColliderComponent>().StaticFriction = staticFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CapsuleColliderComponent::SetDynamicFriction(f32 dynamicFriction)
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        entity.GetComponent<Atom::CapsuleColliderComponent>().DynamicFriction = dynamicFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const glm::vec3& CapsuleColliderComponent::GetCenter()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::CapsuleColliderComponent>().Center;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 CapsuleColliderComponent::GetRadius()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::CapsuleColliderComponent>().Radius;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 CapsuleColliderComponent::GetHeight()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::CapsuleColliderComponent>().Height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 CapsuleColliderComponent::GetRestitution()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::CapsuleColliderComponent>().Restitution;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 CapsuleColliderComponent::GetStaticFriction()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::CapsuleColliderComponent>().StaticFriction;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    f32 CapsuleColliderComponent::GetDynamicFriction()
    {
        Scene* scene = ScriptEngine::GetRunningScene();
        Atom::Entity entity = scene->FindEntityByUUID(m_Entity.GetUUID());
        return entity.GetComponent<Atom::CapsuleColliderComponent>().DynamicFriction;
    }
}
