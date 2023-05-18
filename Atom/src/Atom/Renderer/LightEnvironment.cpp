#include "atompch.h"
#include "LightEnvironment.h"

#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    LightEnvironment::LightEnvironment()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void LightEnvironment::SetEnvironmentMap(const Ref<TextureCube>& environmentMap)
    {
        if (!m_EnvironmentMap || m_EnvironmentMap->GetUUID() != environmentMap->GetUUID())
        {
            m_EnvironmentMap = environmentMap;
            m_IrradianceMap = Renderer::CreateIrradianceMap(m_EnvironmentMap->GetResource(), 32, environmentMap->GetAssetFilepath().stem().string().c_str());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void LightEnvironment::AddDirectionalLight(const glm::vec3& color, const glm::vec3& direction, f32 intensity)
    {
        Light& light = m_Lights.emplace_back();
        light.Color = { color.r, color.g, color.b, 1.0 };
        light.Direction = { direction.x, direction.y, direction.z, 0.0f };
        light.Intensity = intensity;
        light.Type = LightType::DirLight;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void LightEnvironment::AddPointLight(const glm::vec3& color, const glm::vec3& position, f32 intensity, const glm::vec3& attenuationFactors)
    {
        Light& light = m_Lights.emplace_back();
        light.Color = { color.r, color.g, color.b, 1.0 };
        light.Position = { position.x, position.y, position.z, 1.0f };
        light.Intensity = intensity;
        light.AttenuationFactors = attenuationFactors;
        light.Type = LightType::PointLight;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void LightEnvironment::AddSpotlight(const glm::vec3& color, const glm::vec3& position, const glm::vec3& direction, f32 intensity, f32 coneAngle, const glm::vec3& attenuationFactors)
    {
        Light& light = m_Lights.emplace_back();
        light.Color = { color.r, color.g, color.b, 1.0 };
        light.Position = { position.x, position.y, position.z, 1.0f };
        light.Direction = { direction.x, direction.y, direction.z, 0.0f };
        light.Intensity = intensity;
        light.ConeAngle = coneAngle;
        light.AttenuationFactors = attenuationFactors;
        light.Type = LightType::SpotLight;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void LightEnvironment::ClearLights()
    {
        m_Lights.clear();
    }
}