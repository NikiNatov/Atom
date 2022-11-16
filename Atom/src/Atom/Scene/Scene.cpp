#include "atompch.h"
#include "Scene.h"

#include "Atom/Scene/Components.h"
#include "Atom/Renderer/LightEnvironment.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Scene::Scene(const String& name)
        : m_Name(name)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity(m_Registry.create(), this);
        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<SceneHierarchyComponent>();

        return entity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::DeleteEntity(Entity entity)
    {
        auto& shc = entity.GetComponent<SceneHierarchyComponent>();
        Entity currentChild = shc.FirstChild;

        // Delete all children recursively first
        while (currentChild)
        {
            Entity nextSibling = currentChild.GetComponent<SceneHierarchyComponent>().NextSibling;
            DeleteEntity(currentChild);
            currentChild = nextSibling;
        }

        // Fix links between neighbouring entities
        Entity parent = shc.Parent;
        if (parent && parent.GetComponent<SceneHierarchyComponent>().FirstChild == entity)
        {
            Entity nextSibling = shc.NextSibling;
            parent.GetComponent<SceneHierarchyComponent>().FirstChild = nextSibling;

            if (nextSibling)
                nextSibling.GetComponent<SceneHierarchyComponent>().PreviousSibling = {};
        }
        else
        {
            Entity prev = shc.PreviousSibling;
            Entity next = shc.NextSibling;

            if (prev)
                prev.GetComponent<SceneHierarchyComponent>().NextSibling = next;
            if (next)
                next.GetComponent<SceneHierarchyComponent>().PreviousSibling = prev;
        }

        m_Registry.destroy((entt::entity)entity);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnStart()
    {

    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnUpdate(Timestep ts)
    {
        m_EditorCamera.OnUpdate(ts);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnEnd()
    {

    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnEditRender(Ref<SceneRenderer> renderer)
    {
        LightEnvironment lightEnvironment;

        // Sky light
        {
            auto view = m_Registry.view<SkyLightComponent>();
            for (auto entity : view)
            {
                auto& slc = view.get<SkyLightComponent>(entity);

                if (slc.EnvironmentMap && slc.IrradianceMap)
                {
                    lightEnvironment.SetEnvironmentMap(slc.EnvironmentMap, slc.IrradianceMap);
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
                lightEnvironment.AddDirectionalLight(dlc.Color, glm::normalize(-tc.Translation), dlc.Intensity);
            }
        }

        // Point lights
        {
            auto view = m_Registry.view<PointLightComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [plc, tc] = view.get<PointLightComponent, TransformComponent>(entity);
                lightEnvironment.AddPointLight(plc.Color, tc.Translation, plc.Intensity, plc.AttenuationFactors);
            }
        }

        // Spot lights
        {
            auto view = m_Registry.view<SpotLightComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [slc, tc] = view.get<SpotLightComponent, TransformComponent>(entity);
                lightEnvironment.AddSpotlight(slc.Color, tc.Translation, glm::normalize(slc.Direction), slc.Intensity, glm::radians(slc.ConeAngle), slc.AttenuationFactors);
            }
        }

        renderer->BeginScene(m_EditorCamera, lightEnvironment);

        // Submit meshes
        {
            auto view = m_Registry.view<MeshComponent, TransformComponent, SceneHierarchyComponent>();
            for (auto entity : view)
            {
                auto [mc, tc, shc] = view.get<MeshComponent, TransformComponent, SceneHierarchyComponent>(entity);

                if (shc.Parent)
                {
                    auto accumulatedTransform = shc.Parent.GetComponent<TransformComponent>().GetTransform();
                    Entity currentParent = shc.Parent;

                    while (currentParent.GetComponent<SceneHierarchyComponent>().Parent)
                    {
                        currentParent = currentParent.GetComponent<SceneHierarchyComponent>().Parent;
                        accumulatedTransform = accumulatedTransform * currentParent.GetComponent<TransformComponent>().GetTransform();
                    }

                    renderer->SubmitMesh(mc.Mesh, tc.GetTransform() * accumulatedTransform, {});
                }
                else
                    renderer->SubmitMesh(mc.Mesh, tc.GetTransform(), {});
            }
        }

        renderer->Flush();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::OnRuntimeRender(Ref<SceneRenderer> renderer)
    {

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
