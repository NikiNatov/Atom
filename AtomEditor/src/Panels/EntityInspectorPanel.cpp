#include "EntityInspectorPanel.h"

#include <imgui/imgui.h>
#include <entt/entt.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Atom
{
	namespace Utils
	{
		// -----------------------------------------------------------------------------------------------------------------------------
		static void DrawVec3Control(const String& label, glm::vec3& values, f32 resetValue = 0.0f, f32 columnWidth = 100.0f)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::Text(label.c_str());
			ImGui::NextColumn();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			f32 lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			f32 itemWidth = ImGui::GetColumnWidth() * 0.2f;

			// "X" reset button
			ImGui::PushItemWidth(itemWidth);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });

			ImGui::PushFont(boldFont);
			if (ImGui::Button("X", buttonSize))
				values.x = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			// "Y" reset button
			ImGui::PushItemWidth(itemWidth);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize))
				values.y = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			// "Z" reset button
			ImGui::PushItemWidth(itemWidth);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });

			ImGui::PushFont(boldFont);
			if (ImGui::Button("Z", buttonSize))
				values.z = resetValue;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();

			ImGui::Columns(1);

			ImGui::PopID();
		}

		// -----------------------------------------------------------------------------------------------------------------------------
		template<typename ComponentType, typename UIFunction>
		static void DrawComponent(const String& name, Entity entity, bool allowRemove, UIFunction uiFunction)
		{
			ImGui::PushID(entt::type_id<ComponentType>().hash());

			auto& component = entity.GetComponent<ComponentType>();

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_AllowItemOverlap;

			ImVec2 contentRegion = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			f32 lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;

			bool open = ImGui::TreeNodeEx(name.c_str(), flags);

			ImGui::SameLine(contentRegion.x - lineHeight * 0.5f);

			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				ImGui::OpenPopup("Component Options");

			ImGui::PopStyleVar();

			bool removeComponent = false;
			if (allowRemove && ImGui::BeginPopup("Component Options"))
			{
				if (ImGui::MenuItem("Remove"))
					removeComponent = true;
				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<ComponentType>();

			ImGui::PopID();
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	void EntityInspectorPanel::OnImGuiRender()
	{
		ImGui::Begin("Entity Inspector");

		if (m_Entity)
		{
			struct ComponentData
			{
				String ComponentName;
				entt::id_type ComponentHash;
				void(*AddComponentFn)(Entity);
			};

			Vector<ComponentData> missingComponents;
			missingComponents.reserve(10);

			if (m_Entity.HasComponent<TagComponent>())
			{
				Utils::DrawComponent<TagComponent>("Tag", m_Entity, false, [](auto& component)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Tag");
					ImGui::NextColumn();

					char buffer[256];
					memset(buffer, 0, sizeof(buffer));
					strcpy_s(buffer, sizeof(buffer), component.Tag.c_str());

					if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
					{
						component.Tag = buffer;
					}

					ImGui::Columns(1);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Tag";
				data.ComponentHash = entt::type_id<TagComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<TagComponent>(); };
			}

			if (m_Entity.HasComponent<TransformComponent>())
			{
				Utils::DrawComponent<TransformComponent>("Transform", m_Entity, false, [](auto& component)
				{
					Utils::DrawVec3Control("Transform", component.Translation);
					glm::vec3 rotation = glm::degrees(component.Rotation);
					Utils::DrawVec3Control("Rotation", rotation);
					component.Rotation = glm::radians(rotation);
					Utils::DrawVec3Control("Scale", component.Scale, 1.0f);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Transform";
				data.ComponentHash = entt::type_id<TransformComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<TransformComponent>(); };
			}

			if (m_Entity.HasComponent<CameraComponent>())
			{
				Utils::DrawComponent<CameraComponent>("Camera", m_Entity, true, [](auto& component)
				{
					auto& camera = component.Camera;

					const char* projTypesStrings[] = { "Perspective", "Orthographic" };
					const char* currentProjType = projTypesStrings[(s32)camera.GetProjectionType()];

					if (ImGui::BeginCombo("Projection", currentProjType))
					{
						for (u32 i = 0; i < 2; i++)
						{
							bool isSelected = currentProjType == projTypesStrings[i];
							if (ImGui::Selectable(projTypesStrings[i], isSelected))
							{
								currentProjType = projTypesStrings[i];
								camera.SetProjectionType((SceneCamera::ProjectionType)i);
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}
					if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
					{
						f32 fov = camera.GetPerspectiveFOV();
						f32 nearPlane = camera.GetPerspectiveNear();
						f32 farPlane = camera.GetPerspectiveFar();

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Perspective FOV");
						ImGui::NextColumn();
						if (ImGui::DragFloat("##PerspectiveFOV", &fov))
							camera.SetPerspectiveFOV(fov);
						ImGui::Columns(1);

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Perspective Near");
						ImGui::NextColumn();
						if (ImGui::DragFloat("##PerspectiveNear", &nearPlane))
							camera.SetPerspectiveNear(nearPlane);
						ImGui::Columns(1);

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Perspective Far");
						ImGui::NextColumn();
						if (ImGui::DragFloat("##PerspectiveFar", &farPlane))
							camera.SetPerspectiveFar(farPlane);
						ImGui::Columns(1);
					}
					else
					{
						f32 orthoSize = camera.GetOrthographicSize();
						f32 orthoNear = camera.GetOrthographicNear();
						f32 orthoFar = camera.GetOrthographicFar();

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Orthographic Size");
						ImGui::NextColumn();
						if (ImGui::DragFloat("##OrthographicSize", &orthoSize))
							camera.SetOrthographicSize(orthoSize);
						ImGui::Columns(1);

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Orthographic Near");
						ImGui::NextColumn();
						if (ImGui::DragFloat("##OrthographicNear", &orthoNear))
							camera.SetOrthographicNear(orthoNear);
						ImGui::Columns(1);

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Orthographic Far");
						ImGui::NextColumn();
						if (ImGui::DragFloat("##OrthographicFar", &orthoFar))
							camera.SetOrthographicFar(orthoFar);
						ImGui::Columns(1);
					}

					ImGui::Checkbox("Primary", &component.Primary);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Camera";
				data.ComponentHash = entt::type_id<CameraComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<CameraComponent>(); };
			}

			if (m_Entity.HasComponent<MeshComponent>())
			{
				Utils::DrawComponent<MeshComponent>("Mesh", m_Entity, true, [](auto& component)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Mesh");
					ImGui::NextColumn();


					ImGui::InputText("##Mesh", component.Mesh ? (char*)component.Mesh->GetName().c_str() : "Null", 50, ImGuiInputTextFlags_ReadOnly);

					ImGui::Columns(1);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Mesh";
				data.ComponentHash = entt::type_id<MeshComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<MeshComponent>(); };
			}

			if (m_Entity.HasComponent<SkyLightComponent>())
			{
				Utils::DrawComponent<SkyLightComponent>("Sky Light", m_Entity, true, [](auto& component)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Environment");
					ImGui::NextColumn();

					ImGui::InputText("##EnvironmentMap", "EnvironmentMap", 50, ImGuiInputTextFlags_ReadOnly);

					ImGui::Columns(1);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Sky Light";
				data.ComponentHash = entt::type_id<SkyLightComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<SkyLightComponent>(); };
			}

			if (m_Entity.HasComponent<DirectionalLightComponent>())
			{
				Utils::DrawComponent<DirectionalLightComponent>("Directional Light", m_Entity, true, [](auto& component)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Color");
					ImGui::NextColumn();
					ImGui::ColorEdit3("##Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_DisplayRGB);
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Intensity");
					ImGui::NextColumn();
					ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.1f, 5.0f, "%.2f");
					ImGui::Columns(1);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Directional Light";
				data.ComponentHash = entt::type_id<DirectionalLightComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<DirectionalLightComponent>(); };
			}

			if (m_Entity.HasComponent<PointLightComponent>())
			{
				Utils::DrawComponent<PointLightComponent>("Point Light", m_Entity, true, [](auto& component)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Color");
					ImGui::NextColumn();
					ImGui::ColorEdit3("##Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_DisplayRGB);
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Intensity");
					ImGui::NextColumn();
					ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.1f, 5.0f, "%.2f");
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Attenuation");
					ImGui::NextColumn();
					ImGui::DragFloat3("##Attenuation", glm::value_ptr(component.AttenuationFactors), 0.05f, 0.0f, 10.0f, "%.2f");
					ImGui::Columns(1);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Point Light";
				data.ComponentHash = entt::type_id<PointLightComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<PointLightComponent>(); };
			}

			if (m_Entity.HasComponent<SpotLightComponent>())
			{
				Utils::DrawComponent<SpotLightComponent>("Spot Light", m_Entity, true, [](auto& component)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Color");
					ImGui::NextColumn();
					ImGui::ColorEdit3("##Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_DisplayRGB);
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Direction");
					ImGui::NextColumn();
					ImGui::DragFloat3("##Direction", glm::value_ptr(component.Direction), 0.05f, -1.0f, 1.0f, "%.2f");
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Cone Angle");
					ImGui::NextColumn();
					ImGui::DragFloat("##ConeAngle", &component.ConeAngle, 0.05f, 0.0f, 45.0f, "%.2f");
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Intensity");
					ImGui::NextColumn();
					ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.1f, 5.0f, "%.2f");
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 100.0f);
					ImGui::Text("Attenuation");
					ImGui::NextColumn();
					ImGui::DragFloat3("##Attenuation", glm::value_ptr(component.AttenuationFactors), 0.05f, 0.0f, 10.0f, "%.2f");
					ImGui::Columns(1);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Spot Light";
				data.ComponentHash = entt::type_id<SpotLightComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<SpotLightComponent>(); };
			}

			if (!missingComponents.empty())
			{
				if (ImGui::Button("Add Component"))
					ImGui::OpenPopup("##Add Component");

				if (ImGui::BeginPopup("##Add Component"))
				{
					for (const auto& componentData : missingComponents)
					{
						ImGui::PushID(componentData.ComponentHash);

						if (ImGui::Selectable(componentData.ComponentName.data()))
							componentData.AddComponentFn(m_Entity);

						ImGui::PopID();
					}

					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();
	}
}