#include "atompch.h"
#include "LightEnvironment.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void LightEnvironment::SetEnvironmentMap(const Ref<EnvironmentMap>& environmentMap)
    {
        m_EnvironmentMap = environmentMap;
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
}