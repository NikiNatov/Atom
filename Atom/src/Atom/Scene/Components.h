#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/UUID.h"
#include "Atom/Renderer/Camera.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Scene/Entity.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Atom
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent& other) = default;
		IDComponent(UUID uuid)
			: ID(uuid) {}
	};

	struct SceneHierarchyComponent
	{
		UUID Parent = UUID(0);
		UUID FirstChild = UUID(0);
		UUID PreviousSibling = UUID(0);
		UUID NextSibling = UUID(0);

		SceneHierarchyComponent() = default;
		SceneHierarchyComponent(const SceneHierarchyComponent& other) = default;
	};

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

			return translation * rotation * scale;
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
		MeshComponent(Ref<Atom::Mesh> mesh)
			: Mesh(mesh) {}
	};

	struct AnimatedMeshComponent
	{
		Ref<Mesh>	  Mesh = nullptr;
		Ref<Skeleton> Skeleton = nullptr;

		AnimatedMeshComponent() = default;
		AnimatedMeshComponent(const AnimatedMeshComponent& other) = default;
		AnimatedMeshComponent(Ref<Atom::Mesh> mesh, Ref<Atom::Skeleton> skeleton)
			: Mesh(mesh), Skeleton(skeleton) {}
	};

	struct AnimatorComponent
	{
		Ref<AnimationController> AnimationController = nullptr;
		f32						 CurrentTime = 0.0f;
		bool					 Play = false;

		AnimatorComponent() = default;
		AnimatorComponent(const AnimatorComponent& other) = default;
		AnimatorComponent(Ref<Atom::AnimationController> animationController, f32 currentTime, bool play)
			: AnimationController(animationController), CurrentTime(currentTime), Play(play) {}
	};

	struct SkyLightComponent
	{
		Ref<TextureCube> EnvironmentMap = nullptr;

		SkyLightComponent() = default;
		SkyLightComponent(const SkyLightComponent& other) = default;
		SkyLightComponent(Ref<TextureCube> environmentMap)
			: EnvironmentMap(environmentMap) {}
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

	struct ScriptComponent
	{
		String ScriptClass;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent& other) = default;
		ScriptComponent(const String& scriptClass)
			: ScriptClass(scriptClass) {}
	};

	struct RigidbodyComponent
	{
		enum class RigidbodyType { Static, Dynamic };

		RigidbodyType Type = RigidbodyType::Static;
		f32			  Mass = 1.0f;
		glm::bvec3	  FixedRotation = { false, false, false };

		RigidbodyComponent() = default;
		RigidbodyComponent(const RigidbodyComponent& other) = default;
	};

	struct BoxColliderComponent
	{
		glm::vec3 Center = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Size = { 1.0f, 1.0f, 1.0f };
		f32		  Restitution = 0.0f;
		f32		  StaticFriction = 0.5f;
		f32		  DynamicFriction = 0.5f;

		BoxColliderComponent() = default;
		BoxColliderComponent(const BoxColliderComponent& other) = default;
	};

	struct SphereColliderComponent
	{
		glm::vec3 Center = { 0.0f, 0.0f, 0.0f };
		f32		  Radius = 1.0f;
		f32		  Restitution = 0.0f;
		f32		  StaticFriction = 0.5f;
		f32		  DynamicFriction = 0.5f;

		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent& other) = default;
	};
}