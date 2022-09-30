#include "atompch.h"
#include "EditorCamera.h"

#include "Atom/Core/Input.h"
#include <glm/gtc/quaternion.hpp>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    EditorCamera::EditorCamera(f32 fov, f32 aspectRatio, f32 nearPlane, f32 farPlane)
        : PerspectiveCamera(fov, aspectRatio, nearPlane, farPlane)
    {
        m_Distance = glm::distance(m_FocalPoint, m_Position);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    EditorCamera::~EditorCamera()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorCamera::OnUpdate(Timestep ts)
    {
        const glm::vec2 mousePos = Input::GetMousePosition();
        glm::vec2 delta = mousePos - m_LastMousePosition;
        m_LastMousePosition = mousePos;

        if (Input::IsKeyPressed(Key::LAlt))
        {
            if (Input::IsMouseButtonPressed(MouseButton::Middle))
                Pan(delta, ts);
            else if (Input::IsMouseButtonPressed(MouseButton::Left))
                Rotate(delta, ts);
            else if (Input::IsMouseButtonPressed(MouseButton::Right))
                Zoom(delta.y, ts);
        }

        RecalculateViewMatrix();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorCamera::OnEvent(Event& event)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::quat EditorCamera::GetOrientation() const
    {
        return glm::normalize(glm::angleAxis(glm::radians(-m_YawAngle), glm::vec3(0.0f, 1.0f, 0.0f)) *
                              glm::angleAxis(glm::radians(-m_PitchAngle), glm::vec3(1.0f, 0.0f, 0.0f)));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::vec3 EditorCamera::GetCameraUp() const
    {
        return glm::normalize(GetOrientation() * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::vec3 EditorCamera::GetCameraRight() const
    {
        return glm::normalize(GetOrientation() * glm::vec3(1.0f, 0.0f, 0.0f));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::vec3 EditorCamera::GetCameraFront() const
    {
        return glm::normalize(GetOrientation() * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorCamera::RecalculateViewMatrix()
    {
        m_Position = CalculatePosition();
        glm::mat4 rotationMatrix = glm::mat4_cast(glm::conjugate(GetOrientation()));
        m_ViewMatrix = glm::transpose(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * rotationMatrix * glm::translate(glm::mat4(1.0f), -m_Position));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::vec3 EditorCamera::CalculatePosition() const
    {
        return m_FocalPoint - GetCameraFront() * m_Distance;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorCamera::Pan(const glm::vec2& mouseDelta, Timestep ts)
    {
        m_FocalPoint += -GetCameraRight() * mouseDelta.x * m_PanSpeed * (f32)ts;
        m_FocalPoint += GetCameraUp() * mouseDelta.y * m_PanSpeed * (f32)ts;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorCamera::Rotate(const glm::vec2& mouseDelta, Timestep ts)
    {
        f32 sign = GetCameraUp().y < 0.0f ? -1.0f : 1.0f;
        m_YawAngle += sign * mouseDelta.x * m_RotationSpeed * (f32)ts;
        m_PitchAngle += mouseDelta.y * m_RotationSpeed * (f32)ts;

        if (m_PitchAngle > 90.0f)
        {
            m_PitchAngle = 90.0f;
        }
        else if (m_PitchAngle < -90.0f)
        {
            m_PitchAngle = -90.0f;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorCamera::Zoom(f32 zoomDelta, Timestep ts)
    {
        m_Distance -= zoomDelta * m_ZoomSpeed * ts;

        if (m_Distance < 1.0f)
            m_Distance = 1.0f;
    }
}
