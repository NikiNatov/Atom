#include "atompch.h"
#include "Scene.h"

#include "Atom/Scene/Components.h"
#include "Atom/Renderer/LightEnvironment.h"
#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Physics/PhysicsEngine.h"
#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    template<typename Component>
    static void CopyComponent(entt::registry& dstRegistry, const entt::registry& srcRegistry, const HashMap<UUID, entt::entity>& uuidToEnttIDMap)
    {
        for (auto srcEntity : srcRegistry.view<Component>())
        {
            entt::entity dstEntity = uuidToEnttIDMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);
            auto& srcComponent = srcRegistry.get<Component>(srcEntity);
            dstRegistry.emplace_or_replace<Component>(dstEntity, srcComponent);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<typename Component>
    static void CopyComponentIfExists(Entity dstEntity, Entity srcEntity)
    {
        if (srcEntity.HasComponent<Component>())
            dstEntity.AddOrReplaceComponent<Component>(srcEntity.GetComponent<Component>());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Scene::Scene(const String& name)
        : m_Name(name)
    {
        m_LightEnvironment = CreateRef<LightEnvironment>();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Scene> Scene::Copy()
    {
        Ref<Scene> newScene = CreateRef<Scene>(m_Name);
        HashMap<UUID, entt::entity> uuidToEnttIDMap;

        for (auto e : m_Registry.view<IDComponent>())
        {
            UUID uuid = m_Registry.get<IDComponent>(e).ID;
            const String& name = m_Registry.get<TagComponent>(e).Tag;

            // Set the entt id in the new scene registry that corresponds to the UUID of the entity
            uuidToEnttIDMap[uuid] = (entt::entity)newScene->CreateEntityFromUUID(uuid, name);
        }

        CopyComponent<TransformComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);
        CopyComponent<CameraComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);
        CopyComponent<MeshComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);
        CopyComponent<SkyLightComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);
        CopyComponent<DirectionalLightComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);
        CopyComponent<PointLightComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);
        CopyComponent<SpotLightComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);
        CopyComponent<ScriptComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);
        CopyComponent<RigidbodyComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);
        CopyComponent<BoxColliderComponent>(newScene->m_Registry, m_Registry, uuidToEnttIDMap);

        return newScene;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Entity Scene::CreateEntity(const std::string& name)
    {
        return CreateEntityFromUUID(UUID(), name);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Entity Scene::CreateEntityFromUUID(UUID uuid, const String& name)
    {
        Entity entity(m_Registry.create(), this);
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<SceneHierarchyComponent>();
        m_EntitiesByID[uuid] = entity;

        return entity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::DuplicateEntity(Entity entity)
    {
        Entity newEntity = CreateEntity(entity.GetTag());

        CopyComponentIfExists<TransformComponent>(newEntity, entity);
        CopyComponentIfExists<CameraComponent>(newEntity, entity);
        CopyComponentIfExists<MeshComponent>(newEntity, entity);
        CopyComponentIfExists<SkyLightComponent>(newEntity, entity);
        CopyComponentIfExists<DirectionalLightComponent>(newEntity, entity);
        CopyComponentIfExists<PointLightComponent>(newEntity, entity);
        CopyComponentIfExists<SpotLightComponent>(newEntity, entity);
        CopyComponentIfExists<ScriptComponent>(newEntity, entity);
        CopyComponentIfExists<RigidbodyComponent>(newEntity, entity);
        CopyComponentIfExists<BoxColliderComponent>(newEntity, entity);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::DeleteEntity(Entity entity)
    {
        auto& shc = entity.GetComponent<SceneHierarchyComponent>();
        Entity currentChild = FindEntityByUUID(shc.FirstChild);

        // Delete all children recursively first
        while (currentChild)
        {
            Entity nextSibling = FindEntityByUUID(currentChild.GetComponent<SceneHierarchyComponent>().NextSibling);
            DeleteEntity(currentChild);
            currentChild = nextSibling;
        }

        // Fix links between neighbouring entities
        Entity parent = FindEntityByUUID(shc.Parent);
        if (parent && FindEntityByUUID(parent.GetComponent<SceneHierarchyComponent>().FirstChild) == entity)
        {
            Entity nextSibling = FindEntityByUUID(shc.NextSibling);
            parent.GetComponent<SceneHierarchyComponent>().FirstChild = nextSibling.GetUUID();

            if (nextSibling)
                nextSibling.GetComponent<SceneHierarchyComponent>().PreviousSibling = UUID(0);
        }
        else
        {
            Entity prev = FindEntityByUUID(shc.PreviousSibling);
            Entity next = FindEntityByUUID(shc.NextSibling);

            if (prev)
                prev.GetComponent<SceneHierarchyComponent>().NextSibling = next.GetUUID();
            if (next)
                next.GetComponent<SceneHierarchyComponent>().PreviousSibling = prev.GetUUID();
        }

        m_EntitiesByID.erase(entity.GetUUID());
        m_Registry.destroy((entt::entity)entity);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Entity Scene::FindEntityByUUID(UUID uuid)
    {
        auto it = m_EntitiesByID.find(uuid);

        if (it == m_EntitiesByID.end())
            return {};

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Entity Scene::FindEntityByName(const String& name)
    {
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view)
        {
            auto& tag = view.get<TagComponent>(entity);
            if (tag.Tag == name)
                return { entity, this };
        }

        return {};
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnStart()
    {
        m_State = SceneState::Running;

        // Create physics objects
        PhysicsEngine::OnSceneStart(this);

        for (auto entity : m_Registry.view<RigidbodyComponent>())
        {
            PhysicsEngine::CreateRigidbody(Entity(entity, this));
        }

        for (auto entity : m_Registry.view<BoxColliderComponent>())
        {
            PhysicsEngine::CreateBoxCollider(Entity(entity, this));
        }

        // Create script instances
        ScriptEngine::OnSceneStart(this);

        for (auto entity : m_Registry.view<ScriptComponent>())
        {
            ScriptEngine::CreateEntityScript(Entity(entity, this));
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnUpdate(Timestep ts)
    {
        // Update scripts
        auto view = m_Registry.view<ScriptComponent>();
        for (auto entity : view)
        {
            ScriptEngine::UpdateEntityScript(Entity(entity, this), ts);
        }

        for (auto entity : view)
        {
            ScriptEngine::LateUpdateEntityScript(Entity(entity, this), ts);
        }

        // Simulate physics and update transforms
        PhysicsEngine::Simulate(ts);
        for (auto entity : m_Registry.view<RigidbodyComponent>())
        {
            PhysicsEngine::UpdateEntity(Entity(entity, this));
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnStop()
    {
        m_State = SceneState::Edit;

        ScriptEngine::OnSceneStop();
        PhysicsEngine::OnSceneStop();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnEditRender(Ref<SceneRenderer> renderer)
    {
        m_LightEnvironment->ClearLights();

        // Sky light
        {
            auto view = m_Registry.view<SkyLightComponent>();
            for (auto entity : view)
            {
                auto& slc = view.get<SkyLightComponent>(entity);

                if (slc.EnvironmentMap)
                {
                    m_LightEnvironment->SetEnvironmentMap(slc.EnvironmentMap);
                    break;
                }
            }
        }

        // Directional lights
        {
            auto view = m_Registry.view<DirectionalLightComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [dlc, tc] = view.get<DirectionalLightComponent, TransformComponent>(entity);
                m_LightEnvironment->AddDirectionalLight(dlc.Color, glm::normalize(-tc.Translation), dlc.Intensity);
            }
        }

        // Point lights
        {
            auto view = m_Registry.view<PointLightComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [plc, tc] = view.get<PointLightComponent, TransformComponent>(entity);
                m_LightEnvironment->AddPointLight(plc.Color, tc.Translation, plc.Intensity, plc.AttenuationFactors);
            }
        }

        // Spot lights
        {
            auto view = m_Registry.view<SpotLightComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [slc, tc] = view.get<SpotLightComponent, TransformComponent>(entity);
                m_LightEnvironment->AddSpotlight(slc.Color, tc.Translation, glm::normalize(slc.Direction), slc.Intensity, glm::radians(slc.ConeAngle), slc.AttenuationFactors);
            }
        }

        renderer->BeginScene(m_EditorCamera, m_LightEnvironment);

        // Submit meshes
        {
            auto view = m_Registry.view<MeshComponent, TransformComponent, SceneHierarchyComponent>();
            for (auto entity : view)
            {
                auto [mc, tc, shc] = view.get<MeshComponent, TransformComponent, SceneHierarchyComponent>(entity);

                if (mc.Mesh && !mc.Mesh->IsEmpty())
                {
                    if (shc.Parent)
                    {
                        Entity currentParent = FindEntityByUUID(shc.Parent);
                        auto accumulatedTransform = currentParent.GetComponent<TransformComponent>().GetTransform();

                        while (currentParent.GetComponent<SceneHierarchyComponent>().Parent)
                        {
                            currentParent = FindEntityByUUID(currentParent.GetComponent<SceneHierarchyComponent>().Parent);
                            accumulatedTransform = currentParent.GetComponent<TransformComponent>().GetTransform() * accumulatedTransform;
                        }

                        renderer->SubmitMesh(mc.Mesh, accumulatedTransform * tc.GetTransform(), {});
                    }
                    else
                        renderer->SubmitMesh(mc.Mesh, tc.GetTransform(), {});
                }
            }
        }

        renderer->Flush();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnRuntimeRender(Ref<SceneRenderer> renderer)
    {
        auto view = m_Registry.view<CameraComponent, TransformComponent, SceneHierarchyComponent>();

        Camera* mainCamera = nullptr;
        glm::mat4 cameraTransform;

        for (auto entity : view)
        {
            auto [cc, tc, shc] = view.get<CameraComponent, TransformComponent, SceneHierarchyComponent>(entity);

            if (cc.Primary)
            {
                mainCamera = &cc.Camera;

                if (shc.Parent)
                {
                    Entity currentParent = FindEntityByUUID(shc.Parent);
                    auto accumulatedTransform = currentParent.GetComponent<TransformComponent>().GetTransform();

                    while (currentParent.GetComponent<SceneHierarchyComponent>().Parent)
                    {
                        currentParent = FindEntityByUUID(currentParent.GetComponent<SceneHierarchyComponent>().Parent);
                        accumulatedTransform = currentParent.GetComponent<TransformComponent>().GetTransform() * accumulatedTransform;
                    }

                    cameraTransform = accumulatedTransform * tc.GetTransform();
                }
                else
                {
                    cameraTransform = tc.GetTransform();
                }

                break;
            }
        }

        if (mainCamera)
        {
            m_LightEnvironment->ClearLights();

            // Sky light
            {
                auto view = m_Registry.view<SkyLightComponent>();
                for (auto entity : view)
                {
                    auto& slc = view.get<SkyLightComponent>(entity);

                    if (slc.EnvironmentMap)
                    {
                        m_LightEnvironment->SetEnvironmentMap(slc.EnvironmentMap);
                        break;
                    }
                }
            }

            // Directional lights
            {
                auto view = m_Registry.view<DirectionalLightComponent, TransformComponent>();
                for (auto entity : view)
                {
                    auto [dlc, tc] = view.get<DirectionalLightComponent, TransformComponent>(entity);
                    m_LightEnvironment->AddDirectionalLight(dlc.Color, glm::normalize(-tc.Translation), dlc.Intensity);
                }
            }

            // Point lights
            {
                auto view = m_Registry.view<PointLightComponent, TransformComponent>();
                for (auto entity : view)
                {
                    auto [plc, tc] = view.get<PointLightComponent, TransformComponent>(entity);
                    m_LightEnvironment->AddPointLight(plc.Color, tc.Translation, plc.Intensity, plc.AttenuationFactors);
                }
            }

            // Spot lights
            {
                auto view = m_Registry.view<SpotLightComponent, TransformComponent>();
                for (auto entity : view)
                {
                    auto [slc, tc] = view.get<SpotLightComponent, TransformComponent>(entity);
                    m_LightEnvironment->AddSpotlight(slc.Color, tc.Translation, glm::normalize(slc.Direction), slc.Intensity, glm::radians(slc.ConeAngle), slc.AttenuationFactors);
                }
            }

            renderer->BeginScene(*mainCamera, cameraTransform, m_LightEnvironment);

            // Submit meshes
            {
                auto view = m_Registry.view<MeshComponent, TransformComponent, SceneHierarchyComponent>();
                for (auto entity : view)
                {
                    auto [mc, tc, shc] = view.get<MeshComponent, TransformComponent, SceneHierarchyComponent>(entity);

                    if (mc.Mesh && !mc.Mesh->IsEmpty())
                    {
                        if (shc.Parent)
                        {
                            Entity currentParent = FindEntityByUUID(shc.Parent);
                            auto accumulatedTransform = currentParent.GetComponent<TransformComponent>().GetTransform();

                            while (currentParent.GetComponent<SceneHierarchyComponent>().Parent)
                            {
                                currentParent = FindEntityByUUID(currentParent.GetComponent<SceneHierarchyComponent>().Parent);
                                accumulatedTransform = currentParent.GetComponent<TransformComponent>().GetTransform() * accumulatedTransform;
                            }

                            renderer->SubmitMesh(mc.Mesh, accumulatedTransform * tc.GetTransform(), nullptr);
                        }
                        else
                            renderer->SubmitMesh(mc.Mesh, tc.GetTransform(), nullptr);
                    }
                }
            }

            renderer->Flush();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnViewportResize(u32 width, u32 height)
    {
        m_EditorCamera.SetViewport(width, height);

        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
            {
                cameraComponent.Camera.SetViewportSize(width, height);
            }
        }
    }
}
