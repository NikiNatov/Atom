#pragma once

#include "Atom/Core/Core.h"
#include <glm/glm.hpp>

namespace Atom
{
	class Camera
	{
	public:
		Camera() = default;
		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
		glm::mat4& GetProjection() { return m_ProjectionMatrix; }
	protected:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
	};

    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera(f32 fov = 65.0f, f32 aspectRatio = 16.0f / 9.0f, f32 nearPlane = 0.1f, f32 farPlane = 1000.0f);
        ~PerspectiveCamera() = default;

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
    };

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(f32 size = 10.0f, f32 aspectRatio = 16.0f / 9.0f, f32 nearPlane = 1000.0f, f32 farPlane = -1000.0f);
		~OrthographicCamera() = default;

		void SetOrthographic(f32 size, f32 nearPlane, f32 farPlane);
		void SetViewport(u32 width, u32 height);
		inline void SetSize(float size) { m_Size = size; RecalculateProjection(); }
		inline void SetNear(float value) { m_Near = value; RecalculateProjection(); }
		inline void SetFar(float value) { m_Far = value; RecalculateProjection(); }
		inline f32 GetSize() const { return m_Size; }
		inline f32 GetNear() const { return m_Near; }
		inline f32 GetFar() const { return m_Far; }
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	private:
		void RecalculateProjection();
	private:
		f32		  m_Size = 10.0f;
		f32		  m_Near = 1000.0f;
		f32		  m_Far = -1000.0f;
		f32       m_AspectRatio = 16.0f / 9.0f;
	};

	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };
	public:
		SceneCamera();
		~SceneCamera() = default;

		void SetViewportSize(u32 width, u32 height);

		inline void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }
		inline ProjectionType GetProjectionType() const { return m_ProjectionType; }

		void SetPerspective(f32 fov, f32 nearPlane, f32 farPlane);
		inline void SetPerspectiveFOV(float fov) { m_PerspectiveFOV = fov; RecalculateProjection(); }
		inline void SetPerspectiveNear(float value) { m_PerspectiveNear = value; RecalculateProjection(); }
		inline void SetPerspectiveFar(float value) { m_PerspectiveFar = value; RecalculateProjection(); }
		inline f32 GetPerspectiveFOV() const { return m_PerspectiveFOV; }
		inline f32 GetPerspectiveNear() const { return m_PerspectiveNear; }
		inline f32 GetPerspectiveFar() const { return m_PerspectiveFar; }

		void SetOrthographic(f32 size, f32 nearPlane, f32 farPlane);
		inline void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		inline void SetOrthographicNear(float value) { m_OrthographicNear = value; RecalculateProjection(); }
		inline void SetOrthographicFar(float value) { m_OrthographicFar = value; RecalculateProjection(); }
		inline f32 GetOrthographicSize() const { return m_OrthographicSize; }
		inline f32 GetOrthographicNear() const { return m_OrthographicNear; }
		inline f32 GetOrthographicFar() const { return m_OrthographicFar; }
	private:
		void RecalculateProjection();
	private:
		ProjectionType m_ProjectionType = ProjectionType::Perspective;
		f32			   m_AspectRatio = 16.0f / 9.0f;
		f32			   m_PerspectiveFOV = 65.0f;
		f32			   m_PerspectiveNear = 0.1f;
		f32			   m_PerspectiveFar = 1000.0f;
		f32			   m_OrthographicSize = 10.0f;
		f32			   m_OrthographicNear = 1000.0f;
		f32			   m_OrthographicFar = -1000.0f;
	};
}