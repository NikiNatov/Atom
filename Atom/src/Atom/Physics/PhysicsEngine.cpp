#include "atompch.h"
#include "PhysicsEngine.h"

#include "Atom/Scene/Components.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
#include <PxActor.h>
#include <PxRigidBody.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Atom
{
    inline static physx::PxDefaultAllocator s_PhysXAllocator;
    inline static physx::PxDefaultErrorCallback s_PhysXErrorCallback;

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::Initialize()
    {
        ms_PhysXFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_PhysXAllocator, s_PhysXErrorCallback);
        ATOM_ENGINE_ASSERT(ms_PhysXFoundation, "Failed creating PhysX Foundation");

        ms_PhysXPvd = PxCreatePvd(*ms_PhysXFoundation);
        physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        ms_PhysXPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

        ms_PhysX = PxCreatePhysics(PX_PHYSICS_VERSION, *ms_PhysXFoundation, physx::PxTolerancesScale(), true, ms_PhysXPvd);
        ATOM_ENGINE_ASSERT(ms_PhysX, "Failed creating PhysX");

        ms_PhysXDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::Shutdown()
    {
        ms_PhysXDispatcher->release();
        ms_PhysX->release();

        physx::PxPvdTransport* transport = ms_PhysXPvd->getTransport();
        ms_PhysXPvd->release();
        transport->release();

        ms_PhysXFoundation->release();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::OnSceneStart(Scene* scene)
    {
        ms_RunningScene = scene;

        physx::PxSceneDesc sceneDesc(ms_PhysX->getTolerancesScale());
        sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
        sceneDesc.cpuDispatcher = ms_PhysXDispatcher;
        sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

        ms_RunningPhysXScene = ms_PhysX->createScene(sceneDesc);

        if (physx::PxPvdSceneClient* pvdClient = ms_RunningPhysXScene->getScenePvdClient())
        {
            pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::Simulate(Timestep ts)
    {
        ms_RunningPhysXScene->simulate(ts.GetSeconds());
        ms_RunningPhysXScene->fetchResults(true);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::OnSceneStop()
    {
        ms_BoxColliders.clear();
        ms_SphereColliders.clear();
        ms_CapsuleColliders.clear();
        ms_Rigidbodies.clear();
        ms_PhysXMaterials.clear();
        ms_RunningScene = nullptr;

        ms_RunningPhysXScene->release();
        ms_RunningScene = nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::UpdateEntity(Entity entity)
    {
        ATOM_ENGINE_ASSERT(ms_RunningPhysXScene);
        auto& tc = entity.GetComponent<TransformComponent>();

        physx::PxRigidActor* actor = GetRigidBody(entity);
        physx::PxTransform transform = actor->getGlobalPose();
        tc.Translation = { transform.p.x, transform.p.y, transform.p.z };
        tc.Rotation = glm::eulerAngles(glm::quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::CreateRigidbody(Entity entity)
    {
        ATOM_ENGINE_ASSERT(ms_RunningPhysXScene);
        auto& tc = entity.GetComponent<TransformComponent>();
        auto& rbc = entity.GetComponent<RigidbodyComponent>();

        glm::quat rotation(tc.Rotation);
        physx::PxTransform transform(physx::PxVec3(tc.Translation.x, tc.Translation.y, tc.Translation.z), physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w));
        physx::PxRigidActor* actor = nullptr;

        if (rbc.Type == RigidbodyComponent::RigidbodyType::Static)
        {
            actor = ms_PhysX->createRigidStatic(transform);
        }
        else
        {
            actor = ms_PhysX->createRigidDynamic(transform);

            physx::PxRigidDynamic* rb = actor->is<physx::PxRigidDynamic>();
            rb->setMass(rbc.Mass);

            physx::PxRigidDynamicLockFlags lockFlags;
            if (rbc.FixedRotation.x)
                lockFlags |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X;
            if (rbc.FixedRotation.y)
                lockFlags |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
            if (rbc.FixedRotation.z)
                lockFlags |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;

            rb->setRigidDynamicLockFlags(lockFlags);
        }

        ms_RunningPhysXScene->addActor(*actor);
        ms_Rigidbodies[entity.GetUUID()] = actor;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::CreateBoxCollider(Entity entity)
    {
        ATOM_ENGINE_ASSERT(ms_RunningPhysXScene);
        auto& bcc = entity.GetComponent<BoxColliderComponent>();

        if (physx::PxRigidActor* rb = GetRigidBody(entity))
        {
            physx::PxMaterial* material = ms_PhysX->createMaterial(bcc.StaticFriction, bcc.DynamicFriction, bcc.Restitution);
            ms_PhysXMaterials[entity.GetUUID()] = material;

            auto& tc = entity.GetComponent<TransformComponent>();
            physx::PxShape* collider = ms_PhysX->createShape(physx::PxBoxGeometry(bcc.Size.x / 2.0f * tc.Scale.x, bcc.Size.y / 2.0f * tc.Scale.y, bcc.Size.z / 2.0f * tc.Scale.z), *material);
            collider->setLocalPose(physx::PxTransform(bcc.Center.x, bcc.Center.y, bcc.Center.z));
            rb->attachShape(*collider);
            ms_BoxColliders[entity.GetUUID()] = collider;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::CreateSphereCollider(Entity entity)
    {
        ATOM_ENGINE_ASSERT(ms_RunningPhysXScene);
        auto& scc = entity.GetComponent<SphereColliderComponent>();

        if (physx::PxRigidActor* rb = GetRigidBody(entity))
        {
            physx::PxMaterial* material = ms_PhysX->createMaterial(scc.StaticFriction, scc.DynamicFriction, scc.Restitution);
            ms_PhysXMaterials[entity.GetUUID()] = material;

            auto& tc = entity.GetComponent<TransformComponent>();
            physx::PxShape* collider = ms_PhysX->createShape(physx::PxSphereGeometry(scc.Radius * glm::max(glm::max(tc.Scale.x, tc.Scale.y), tc.Scale.z)), *material);
            collider->setLocalPose(physx::PxTransform(scc.Center.x, scc.Center.y, scc.Center.z));
            rb->attachShape(*collider);
            ms_SphereColliders[entity.GetUUID()] = collider;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PhysicsEngine::CreateCapsuleCollider(Entity entity)
    {
        ATOM_ENGINE_ASSERT(ms_RunningPhysXScene);
        auto& ccc = entity.GetComponent<CapsuleColliderComponent>();

        if (physx::PxRigidActor* rb = GetRigidBody(entity))
        {
            physx::PxMaterial* material = ms_PhysX->createMaterial(ccc.StaticFriction, ccc.DynamicFriction, ccc.Restitution);
            ms_PhysXMaterials[entity.GetUUID()] = material;

            auto& tc = entity.GetComponent<TransformComponent>();
            physx::PxShape* collider = ms_PhysX->createShape(physx::PxCapsuleGeometry(ccc.Radius * glm::max(tc.Scale.x, tc.Scale.z), ccc.Height / 2.0f * tc.Scale.y), *material);
            // Rotation is needed to make capsule colliders Y-axis aligned (they are X-aligned by default)
            glm::quat rotation(glm::vec3(0.0f, 0.0f, glm::radians(90.0f)));
            collider->setLocalPose(physx::PxTransform(ccc.Center.x, ccc.Center.y, ccc.Center.z, physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)));
            rb->attachShape(*collider);
            ms_CapsuleColliders[entity.GetUUID()] = collider;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxRigidActor* PhysicsEngine::GetRigidBody(Entity entity)
    {
        ATOM_ENGINE_ASSERT(entity);
        return GetRigidBody(entity.GetUUID());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxRigidActor* PhysicsEngine::GetRigidBody(UUID uuid)
    {
        auto it = ms_Rigidbodies.find(uuid);

        if (it == ms_Rigidbodies.end())
            return nullptr;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxShape* PhysicsEngine::GetBoxCollider(Entity entity)
    {
        ATOM_ENGINE_ASSERT(entity);
        return GetBoxCollider(entity.GetUUID());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxShape* PhysicsEngine::GetBoxCollider(UUID uuid)
    {
        auto it = ms_BoxColliders.find(uuid);

        if (it == ms_BoxColliders.end())
            return nullptr;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxShape* PhysicsEngine::GetShepreCollider(Entity entity)
    {
        ATOM_ENGINE_ASSERT(entity);
        return GetShepreCollider(entity.GetUUID());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxShape* PhysicsEngine::GetShepreCollider(UUID uuid)
    {
        auto it = ms_SphereColliders.find(uuid);

        if (it == ms_SphereColliders.end())
            return nullptr;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxShape* PhysicsEngine::GetCapsuleCollider(Entity entity)
    {
        ATOM_ENGINE_ASSERT(entity);
        return GetCapsuleCollider(entity.GetUUID());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxShape* PhysicsEngine::GetCapsuleCollider(UUID uuid)
    {
        auto it = ms_CapsuleColliders.find(uuid);

        if (it == ms_CapsuleColliders.end())
            return nullptr;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxMaterial* PhysicsEngine::GetPhysicsMaterial(Entity entity)
    {
        ATOM_ENGINE_ASSERT(entity);
        return GetPhysicsMaterial(entity.GetUUID());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    physx::PxMaterial* PhysicsEngine::GetPhysicsMaterial(UUID uuid)
    {
        auto it = ms_PhysXMaterials.find(uuid);

        if (it == ms_PhysXMaterials.end())
            return nullptr;

        return it->second;
    }
}
