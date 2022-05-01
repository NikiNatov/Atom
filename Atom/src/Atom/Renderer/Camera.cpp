#include "atompch.h"
#include "Camera.h"

#include <glm\gtc\matrix_transform.hpp>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    PerspectiveCamera::PerspectiveCamera(f32 fov, f32 aspectRatio, f32 nearPlane, f32 farPlane)
        : m_Fov(fov), m_AspectRatio(aspectRatio), m_Near(nearPlane), m_Far(farPlane)
    {
        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    PerspectiveCamera::~PerspectiveCamera()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PerspectiveCamera::SetPerspective(f32 fov, f32 nearPlane, f32 farPlane)
    {
        m_Fov = fov;
        m_Near = nearPlane;
        m_Far = farPlane;

        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PerspectiveCamera::SetViewport(u32 width, u32 height)
    {
        if (height == 0)
        {
            return;
        }

        m_AspectRatio = (f32)width / (f32)height;
        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PerspectiveCamera::RecalculateProjection()
    {
        m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_Near, m_Far);
    }
}
