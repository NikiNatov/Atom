#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"

#include <glm/glm.hpp>

namespace Atom
{
    enum class LightType : u32
    {
        DirLight,
        PointLight,
        SpotLight
    };

    struct Light
    {
        glm::vec4 Position;
        glm::vec4 Direction;
        glm::vec4 Color;
        f32       Intensity;
        f32       ConeAngle;
        glm::vec3 AttenuationFactors;
        LightType Type;
    };

    class LightEnvironment
    {
    public:
        LightEnvironment();
        ~LightEnvironment() = default;

        void SetEnvironmentMap(const Ref<TextureCube>& environmentMap);
        void AddDirectionalLight(const glm::vec3& color, const glm::vec3& direction, f32 intensity);
        void AddPointLight(const glm::vec3& color, const glm::vec3& position, f32 intensity, const glm::vec3& attenuationFactors);
        void AddSpotlight(const glm::vec3& color, const glm::vec3& position, const glm::vec3& direction, f32 intensity, f32 coneAngle, const glm::vec3& attenuationFactors);
        void ClearLights();

        inline Ref<TextureCube> GetEnvironmentMap() const { return m_EnvironmentMap; }
        inline Ref<TextureCube> GetIrradianceMap() const { return m_IrradianceMap; }
        inline const Vector<Light>& GetLights() const { return m_Lights; }
    private:
        Ref<TextureCube> m_EnvironmentMap;
        Ref<TextureCube> m_IrradianceMap;
        Vector<Light>    m_Lights;
    };
}