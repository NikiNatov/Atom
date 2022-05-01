#pragma once

#include "Atom/Core/Core.h"
#include <glm/glm.hpp>

namespace Atom
{
    class PerspectiveCamera
    {
    public:
        PerspectiveCamera(f32 fov = 65.0f, f32 aspectRatio = 16.0f / 9.0f, f32 nearPlane = 0.1f, f32 farPlane = 1000.0f);
        ~PerspectiveCamera();

        void SetPerspective(f32 fov, f32 nearPlane, f32 farPlane);
        void SetViewport(u32 width, u32 height);
        inline void SetFov(f32 fov) { m_Fov = fov; RecalculateProjection(); }
        inline void SetNear(f32 nearPlane) { m_Near = nearPlane; RecalculateProjection(); }
        inline void SetFar(f32 farPlane) { m_Near = farPlane; RecalculateProjection(); }
        inline f32 GetFov() const { return m_Fov; }
        inline f32 GetNear() const { return m_Near; }
        inline f32 GetFar() const { return m_Far; }
        inline f32 GetAspectRatio() const { return m_AspectRatio; }
        inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
    private:
        void RecalculateProjection();
    private:
        f32       m_Fov = 65.0f;
        f32       m_Near = 0.1f;
        f32       m_Far = 1000.0f;
        f32       m_AspectRatio = 16.0f / 9.0f;
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0);
    };
}