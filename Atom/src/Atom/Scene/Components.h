#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Camera.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/Texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Atom
{
    struct TagComponent
    {
        String Tag = "";

		TagComponent() = default;
		TagComponent(const TagComponent& other) = default;
		TagComponent(const String& tag)
            : Tag(tag) {}
    };

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent& other) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform()
		{
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), Translation);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), Scale);
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.z, { 0.0f, 0.0f, 1.0f }) *
								 glm::rotate(glm::mat4(1.0f), Rotation.y, { 0.0f, 1.0f, 0.0f }) *
								 glm::rotate(glm::mat4(1.0f), Rotation.x, { 1.0f, 0.0f, 0.0f });

			return glm::transpose(translation * rotation * scale);
		}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool		Primary = false;
		bool		FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other) = default;
	};

	struct MeshComponent
	{
		Ref<Mesh> Mesh = nullptr;

		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;
		MeshComponent(const Ref<Atom::Mesh>& mesh)
			: Mesh(mesh) {}
	};

	struct SkyLightComponent
	{
		Ref<TextureCube> EnvironmentMap = nullptr;
		Ref<TextureCube> IrradianceMap = nullptr;

		SkyLightComponent() = default;
		SkyLightComponent(const SkyLightComponent& other) = default;
		SkyLightComponent(const Ref<TextureCube>& environmentMap, const Ref<TextureCube>& irradianceMap)
			: EnvironmentMap(environmentMap), IrradianceMap(irradianceMap) {}
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Color = glm::vec3(1.0f);
		f32		  Intensity = 1.0f;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent& other) = default;
		DirectionalLightComponent(const glm::vec3& color, f32 intensity)
			: Color(color), Intensity(intensity) {}
	};

	struct PointLightComponent
	{
		glm::vec3 Color = glm::vec3(1.0f);
		f32		  Intensity = 1.0f;
		glm::vec3 AttenuationFactors = { 1.0f, 1.0f, 1.0f };

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent& other) = default;
		PointLightComponent(const glm::vec3& color, f32 intensity, const glm::vec3& attenuationFactors = glm::vec3(1.0f, 1.0f, 1.0f))
			: Color(color), Intensity(intensity), AttenuationFactors(attenuationFactors) {}
	};

	struct SpotLightComponent
	{
		glm::vec3 Color = glm::vec3(1.0f);
		glm::vec3 Direction = glm::vec3(0.0f);
		f32		  ConeAngle = 30.0f;
		f32		  Intensity = 1.0f;
		glm::vec3 AttenuationFactors = { 1.0f, 1.0f, 1.0f };

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent& other) = default;
		SpotLightComponent(const glm::vec3& color, const glm::vec3& direction, f32 coneAngle, f32 intensity, const glm::vec3& attenuationFactors = glm::vec3(1.0f, 1.0f, 1.0f))
			: Color(color), Direction(direction), ConeAngle(coneAngle), Intensity(intensity), AttenuationFactors(attenuationFactors) {}
	};
}