#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/Timestep.h"
#include "Atom/Core/Events/Event.h"
#include "Camera.h"

namespace Atom
{
    class EditorCamera : public PerspectiveCamera
    {
    public:
        EditorCamera(f32 fov = 65.0f, f32 aspectRatio = 16.0f / 9.0f, f32 nearPlane = 0.1f, f32 farPlane = 1000.0f);
        ~EditorCamera();

        void OnUpdate(Timestep ts);
        void OnEvent(Event& event);
        inline void SetDistance(f32 distance) { m_Distance = distance; }

        inline f32 GetYawAngle() const { return m_YawAngle; }
        inline f32 GetPitchAngle() const { return m_PitchAngle; }
        inline f32 GetDistance() const { return m_Distance; }
        inline const glm::vec3& GetPosition() const { return m_Position; }
        inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

        glm::quat GetOrientation() const;
        glm::vec3 GetCameraUp() const;
        glm::vec3 GetCameraRight() const;
        glm::vec3 GetCameraFront() const;
    private:
        void RecalculateViewMatrix();
        glm::vec3 CalculatePosition() const;
        void Pan(const glm::vec2& mouseDelta, Timestep ts);
        void Rotate(const glm::vec2& mouseDelta, Timestep ts);
        void Zoom(f32 zoomDelta, Timestep ts);
    private:
        f32       m_YawAngle = 0.0f;
        f32       m_PitchAngle = 0.0f;
        f32       m_Distance = 0.0f;
        f32       m_PanSpeed = 1.0f;
        f32       m_RotationSpeed = 30.0f;
        f32       m_ZoomSpeed = 3.0f;
        glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 m_FocalPoint = glm::vec3(0.0f);
        glm::vec2 m_LastMousePosition = glm::vec2(0.0f);
        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
    };
}