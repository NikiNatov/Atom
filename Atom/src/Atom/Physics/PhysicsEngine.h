#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Scene/Scene.h"

namespace physx
{
    class PxFoundation;
    class PxPhysics;
    class PxDefaultCpuDispatcher;
    class PxPvd;
    class PxRigidActor;
    class PxShape;
    class PxScene;
    class PxMaterial;
}

namespace Atom
{
    class PhysicsEngine
    {
    public:
        static void Initialize();
        static void Shutdown();

        static void OnSceneStart(Scene* scene);
        static void Simulate(Timestep ts);
        static void OnSceneStop();

        static void UpdateEntity(Entity entity);

        static void CreateRigidbody(Entity entity);
        static void CreateBoxCollider(Entity entity);
        static void CreateSphereCollider(Entity entity);
        static void CreateCapsuleCollider(Entity entity);

        static physx::PxRigidActor* GetRigidBody(Entity entity);
        static physx::PxRigidActor* GetRigidBody(UUID uuid);
        static physx::PxShape* GetBoxCollider(Entity entity);
        static physx::PxShape* GetBoxCollider(UUID uuid);
        static physx::PxShape* GetShepreCollider(Entity entity);
        static physx::PxShape* GetShepreCollider(UUID uuid);
        static physx::PxShape* GetCapsuleCollider(Entity entity);
        static physx::PxShape* GetCapsuleCollider(UUID uuid);
        static physx::PxMaterial* GetPhysicsMaterial(Entity entity);
        static physx::PxMaterial* GetPhysicsMaterial(UUID uuid);
    private:
        inline static Scene*                              ms_RunningScene = nullptr;
        inline static physx::PxScene*                     ms_RunningPhysXScene = nullptr;
        inline static physx::PxFoundation*                ms_PhysXFoundation = nullptr;
        inline static physx::PxPhysics*                   ms_PhysX = nullptr;
        inline static physx::PxDefaultCpuDispatcher*      ms_PhysXDispatcher = nullptr;
        inline static physx::PxPvd*                       ms_PhysXPvd = nullptr;
        inline static HashMap<UUID, physx::PxRigidActor*> ms_Rigidbodies;
        inline static HashMap<UUID, physx::PxShape*>      ms_BoxColliders;
        inline static HashMap<UUID, physx::PxShape*>      ms_SphereColliders;
        inline static HashMap<UUID, physx::PxShape*>      ms_CapsuleColliders;
        inline static HashMap<UUID, physx::PxMaterial*>   ms_PhysXMaterials;
    };
}