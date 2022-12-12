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

    class EnvironmentMap
    {
    public:
        EnvironmentMap() = default;
        EnvironmentMap(const Ref<TextureCube>& environmentTexture, const Ref<TextureCube>& irradianceTexture)
            : m_EnvironmentTexture(environmentTexture), m_IrradianceTexture(irradianceTexture) {}

        inline Ref<TextureCube> GetEnvironmentTexture() const { return m_EnvironmentTexture; }
        inline Ref<TextureCube> GetIrradianceTexture() const { return m_IrradianceTexture; }
    private:
        Ref<TextureCube> m_EnvironmentTexture;
        Ref<TextureCube> m_IrradianceTexture;
    };

    class LightEnvironment
    {
    public:
        LightEnvironment() = default;
        ~LightEnvironment() = default;

        void SetEnvironmentMap(const Ref<EnvironmentMap>& environmentMap);
        void AddDirectionalLight(const glm::vec3& color, const glm::vec3& direction, f32 intensity);
        void AddPointLight(const glm::vec3& color, const glm::vec3& position, f32 intensity, const glm::vec3& attenuationFactors);
        void AddSpotlight(const glm::vec3& color, const glm::vec3& position, const glm::vec3& direction, f32 intensity, f32 coneAngle, const glm::vec3& attenuationFactors);

        inline Ref<EnvironmentMap> GetEnvironmentMap() const { return m_EnvironmentMap; }
        inline const Vector<Light>& GetLights() const { return m_Lights; }
    private:
        Ref<EnvironmentMap> m_EnvironmentMap;
        Vector<Light>       m_Lights;
    };
}