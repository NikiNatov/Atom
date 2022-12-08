#include "atompch.h"
#include "SceneSerializer.h"

#include "Atom/Scene/Components.h"

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace YAML
{
	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

namespace Atom
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

    // -----------------------------------------------------------------------------------------------------------------------------
    SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
        : m_Scene(scene)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneSerializer::Serialize(const std::filesystem::path& filepath)
    {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << m_Scene->GetName();
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.get() };

			if (!entity)
				return;

			out << YAML::BeginMap;
			out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

			if (entity.HasComponent<TagComponent>())
			{
				auto& tc = entity.GetComponent<TagComponent>();
				out << YAML::Key << "TagComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "Tag" << YAML::Value << tc.Tag;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<SceneHierarchyComponent>())
			{
				auto& shc = entity.GetComponent<SceneHierarchyComponent>();
				out << YAML::Key << "SceneHierarchyComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "Parent" << YAML::Value << shc.Parent;
				out << YAML::Key << "FirstChild" << YAML::Value << shc.FirstChild;
				out << YAML::Key << "NextSibling" << YAML::Value << shc.NextSibling;
				out << YAML::Key << "PreviousSibling" << YAML::Value << shc.PreviousSibling;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<TransformComponent>())
			{
				auto& tc = entity.GetComponent<TransformComponent>();
				out << YAML::Key << "TransformComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
				out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
				out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<CameraComponent>())
			{
				auto& cc = entity.GetComponent<CameraComponent>();
				out << YAML::Key << "CameraComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "Camera" << YAML::Value;
				out << YAML::BeginMap;
				out << YAML::Key << "ProjectionType" << YAML::Value << (s32)cc.Camera.GetProjectionType();
				out << YAML::Key << "PerspectiveFOV" << YAML::Value << cc.Camera.GetPerspectiveFOV();
				out << YAML::Key << "PerspectiveNear" << YAML::Value << cc.Camera.GetPerspectiveNear();
				out << YAML::Key << "PerspectiveFar" << YAML::Value << cc.Camera.GetPerspectiveFar();
				out << YAML::Key << "OrthographicSize" << YAML::Value << cc.Camera.GetOrthographicSize();
				out << YAML::Key << "OrthographicNear" << YAML::Value << cc.Camera.GetOrthographicNear();
				out << YAML::Key << "OrthographicFar" << YAML::Value << cc.Camera.GetOrthographicFar();
				out << YAML::EndMap;
				out << YAML::Key << "Primary" << YAML::Value << cc.Primary;
				out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.FixedAspectRatio;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<MeshComponent>())
			{
				auto& mc = entity.GetComponent<MeshComponent>();
				out << YAML::Key << "MeshComponent";
				out << YAML::BeginMap;
				//out << YAML::Key << "Mesh" << YAML::Value << mc.Mesh->GetName();
				out << YAML::EndMap;
			}

			if (entity.HasComponent<SkyLightComponent>())
			{
				auto& slc = entity.GetComponent<SkyLightComponent>();
				out << YAML::Key << "SkyLightComponent";
				out << YAML::BeginMap;
				//out << YAML::Key << "EnvironmentMap" << YAML::Value << slc.EnvironmentMap;
				//out << YAML::Key << "IrradianceMap" << YAML::Value << slc.IrradianceMap;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<DirectionalLightComponent>())
			{
				auto& dlc = entity.GetComponent<DirectionalLightComponent>();
				out << YAML::Key << "DirectionalLightComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "Color" << YAML::Value << dlc.Color;
				out << YAML::Key << "Intensity" << YAML::Value << dlc.Intensity;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<PointLightComponent>())
			{
				auto& plc = entity.GetComponent<PointLightComponent>();
				out << YAML::Key << "PointLightComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "Color" << YAML::Value << plc.Color;
				out << YAML::Key << "Intensity" << YAML::Value << plc.Intensity;
				out << YAML::Key << "AttenuationFactors" << YAML::Value << plc.AttenuationFactors;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<SpotLightComponent>())
			{
				auto& slc = entity.GetComponent<SpotLightComponent>();
				out << YAML::Key << "SpotLightComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "Color" << YAML::Value << slc.Color;
				out << YAML::Key << "Direction" << YAML::Value << slc.Direction;
				out << YAML::Key << "ConeAngle" << YAML::Value << slc.ConeAngle;
				out << YAML::Key << "Intensity" << YAML::Value << slc.Intensity;
				out << YAML::Key << "AttenuationFactors" << YAML::Value << slc.AttenuationFactors;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<ScriptComponent>())
			{
				auto& sc = entity.GetComponent<ScriptComponent>();
				out << YAML::Key << "ScriptComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "ScriptClass" << YAML::Value << sc.ScriptClass;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<RigidbodyComponent>())
			{
				auto& rbc = entity.GetComponent<RigidbodyComponent>();
				out << YAML::Key << "RigidbodyComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "Type" << YAML::Value << (s32)rbc.Type;
				out << YAML::Key << "Mass" << YAML::Value << rbc.Mass;
				out << YAML::Key << "FixedRotation" << YAML::Value << rbc.FixedRotation;
				out << YAML::EndMap;
			}

			if (entity.HasComponent<BoxColliderComponent>())
			{
				auto& bcc = entity.GetComponent<BoxColliderComponent>();
				out << YAML::Key << "BoxColliderComponent";
				out << YAML::BeginMap;
				out << YAML::Key << "Center" << YAML::Value << bcc.Center;
				out << YAML::Key << "Size" << YAML::Value << bcc.Size;
				out << YAML::Key << "Restitution" << YAML::Value << bcc.Restitution;
				out << YAML::Key << "StaticFriction" << YAML::Value << bcc.StaticFriction;
				out << YAML::Key << "DynamicFriction" << YAML::Value << bcc.DynamicFriction;
				out << YAML::EndMap;
			}

			out << YAML::EndMap;
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool SceneSerializer::Deserialize(const std::filesystem::path& filepath)
    {
		std::ifstream stream(filepath);
		std::stringstream strStream;

		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());

		if (!data["Scene"])
			return false;

		String sceneName = data["Scene"].as<String>();
		ATOM_INFO("Deserializing scene '{0}'", sceneName);

		if (YAML::Node entities = data["Entities"])
		{
			for (s64 it = entities.size() - 1; it >= 0; it--)
			{
				UUID uuid = entities[it]["Entity"].as<u64>();
				YAML::Node tagComponent = entities[it]["TagComponent"];
				String entityName = tagComponent ? tagComponent["Tag"].as<String>() : "Unnamed";

				ATOM_INFO("Deserializing entity with UUID = '{0}', name = {1}", uuid, entityName);

				Entity deserializedEntity = m_Scene->CreateEntityFromUUID(uuid, entityName);

				if (YAML::Node sceneNodeComponent = entities[it]["SceneHierarchyComponent"])
				{
					auto& shc = deserializedEntity.GetComponent<SceneHierarchyComponent>();
					shc.Parent = sceneNodeComponent["Parent"].as<u64>();
					shc.FirstChild = sceneNodeComponent["FirstChild"].as<u64>();
					shc.NextSibling = sceneNodeComponent["NextSibling"].as<u64>();
					shc.PreviousSibling = sceneNodeComponent["PreviousSibling"].as<u64>();
				}

				if (YAML::Node transformComponent = entities[it]["TransformComponent"])
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				if (YAML::Node cameraComponent = entities[it]["CameraComponent"])
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();
					YAML::Node cameraProperties = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProperties["ProjectionType"].as<s32>());
					cc.Camera.SetPerspectiveFOV(cameraProperties["PerspectiveFOV"].as<f32>());
					cc.Camera.SetPerspectiveNear(cameraProperties["PerspectiveNear"].as<f32>());
					cc.Camera.SetPerspectiveFar(cameraProperties["PerspectiveFar"].as<f32>());
					cc.Camera.SetOrthographicSize(cameraProperties["OrthographicSize"].as<f32>());
					cc.Camera.SetOrthographicNear(cameraProperties["OrthographicNear"].as<f32>());
					cc.Camera.SetOrthographicFar(cameraProperties["OrthographicFar"].as<f32>());
					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				if (YAML::Node meshComponent = entities[it]["MeshComponent"])
				{
					auto& mc = deserializedEntity.AddComponent<MeshComponent>();
				}

				if (YAML::Node skyLightComponent = entities[it]["SkyLightComponent"])
				{
					auto& slc = deserializedEntity.AddComponent<SkyLightComponent>();
				}

				if (YAML::Node dirLightComponent = entities[it]["DirectionalLightComponent"])
				{
					auto& dlc = deserializedEntity.AddComponent<DirectionalLightComponent>();
					dlc.Color = dirLightComponent["Color"].as<glm::vec3>();
					dlc.Intensity = dirLightComponent["Intensity"].as<f32>();
				}

				if (YAML::Node pointLightComponent = entities[it]["PointLightComponent"])
				{
					auto& plc = deserializedEntity.AddComponent<PointLightComponent>();
					plc.Color = pointLightComponent["Color"].as<glm::vec3>();
					plc.Intensity = pointLightComponent["Intensity"].as<f32>();
					plc.AttenuationFactors = pointLightComponent["AttenuationFactors"].as<glm::vec3>();
				}

				if (YAML::Node spotLightComponent = entities[it]["SpotLightComponent"])
				{
					auto& slc = deserializedEntity.AddComponent<SpotLightComponent>();
					slc.Color = spotLightComponent["Color"].as<glm::vec3>();
					slc.Direction = spotLightComponent["Direction"].as<glm::vec3>();
					slc.ConeAngle = spotLightComponent["ConeAngle"].as<f32>();
					slc.Intensity = spotLightComponent["Intensity"].as<f32>();
					slc.AttenuationFactors = spotLightComponent["AttenuationFactors"].as<glm::vec3>();
				}

				if (YAML::Node scriptComponent = entities[it]["ScriptComponent"])
				{
					auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
					sc.ScriptClass = scriptComponent["ScriptClass"].as<String>();
				}

				if (YAML::Node ribidbodyComponent = entities[it]["RigidbodyComponent"])
				{
					auto& rbc = deserializedEntity.AddComponent<RigidbodyComponent>();
					rbc.Type = (RigidbodyComponent::RigidbodyType)ribidbodyComponent["Type"].as<s32>();
					rbc.Mass = ribidbodyComponent["Mass"].as<f32>();
					rbc.FixedRotation = ribidbodyComponent["FixedRotation"].as<glm::vec3>();
				}

				if (YAML::Node boxColliderComponent = entities[it]["BoxColliderComponent"])
				{
					auto& bcc = deserializedEntity.AddComponent<BoxColliderComponent>();
					bcc.Center = boxColliderComponent["Center"].as<glm::vec3>();
					bcc.Size = boxColliderComponent["Size"].as<glm::vec3>();
					bcc.Restitution = boxColliderComponent["Restitution"].as<f32>();
					bcc.StaticFriction = boxColliderComponent["StaticFriction"].as<f32>();
					bcc.DynamicFriction = boxColliderComponent["DynamicFriction"].as<f32>();
				}
			}
		}

		return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneSerializer::SetScene(const Ref<Scene>& scene)
    {
        m_Scene = scene;
    }
}
