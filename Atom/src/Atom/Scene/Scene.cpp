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

        return entity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Scene::DeleteEntity(Entity entity)
    {
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
            auto view = m_Registry.view<MeshComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto [mc, tc] = view.get<MeshComponent, TransformComponent>(entity);
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
