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
        m_ProjectionMatrix = glm::perspectiveRH_ZO(glm::radians(m_Fov), m_AspectRatio, m_Near, m_Far);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    OrthographicCamera::OrthographicCamera(f32 size, f32 aspectRatio, f32 nearPlane, f32 farPlane)
        : m_Size(size), m_AspectRatio(aspectRatio), m_Near(nearPlane), m_Far(farPlane)
    {
        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void OrthographicCamera::SetOrthographic(f32 size, f32 nearPlane, f32 farPlane)
    {
        m_Size = size;
        m_Near = nearPlane;
        m_Far = farPlane;

        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void OrthographicCamera::SetViewport(u32 width, u32 height)
    {
        if (height == 0)
        {
            return;
        }

        m_AspectRatio = (f32)width / (f32)height;
        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void OrthographicCamera::RecalculateProjection()
    {
        f32 left = -m_Size * m_AspectRatio * 0.5f;
        f32 right = m_Size * m_AspectRatio * 0.5f;
        f32 top = m_Size * 0.5f;
        f32 bottom = -m_Size * 0.5f;

        m_ProjectionMatrix = glm::orthoRH_ZO(left, right, bottom, top, m_Near, m_Far);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SceneCamera::SceneCamera()
    {
        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneCamera::SetPerspective(f32 fov, f32 nearPlane, f32 farPlane)
    {
        m_PerspectiveFOV = fov;
        m_PerspectiveNear = nearPlane;
        m_PerspectiveFar = farPlane;

        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneCamera::SetOrthographic(f32 size, f32 nearPlane, f32 farPlane)
    {
        m_OrthographicSize = size;
        m_OrthographicNear = nearPlane;
        m_OrthographicFar = farPlane;

        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneCamera::SetViewportSize(u32 width, u32 height)
    {
        if (height == 0)
            return;

        m_AspectRatio = (f32)width / (f32)height;
        RecalculateProjection();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneCamera::RecalculateProjection()
    {
        if (m_ProjectionType == ProjectionType::Perspective)
            m_ProjectionMatrix = glm::perspectiveRH_ZO(glm::radians(m_PerspectiveFOV), m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
        else
        {
            f32 left = -m_OrthographicSize * m_AspectRatio * 0.5f;
            f32 right = m_OrthographicSize * m_AspectRatio * 0.5f;
            f32 top = m_OrthographicSize * 0.5f;
            f32 bottom = -m_OrthographicSize * 0.5f;

            m_ProjectionMatrix = glm::orthoRH_ZO(left, right, bottom, top, m_OrthographicNear, m_OrthographicFar);
        }
    }
}
