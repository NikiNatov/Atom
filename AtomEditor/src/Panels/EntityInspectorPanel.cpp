#include "EntityInspectorPanel.h"
#include "../EditorLayer.h"

#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Scripting/ScriptWrappers/Scene/EntityWrapper.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Atom
{
	namespace Utils
	{
		// -----------------------------------------------------------------------------------------------------------------------------
		static void DrawVec3Control(const String& label, glm::vec3& values, f32 resetValue = 0.0f, f32 columnWidth = 150.0f)
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
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Tag");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					char buffer[256];
					memset(buffer, 0, sizeof(buffer));
					strcpy_s(buffer, sizeof(buffer), component.Tag.c_str());

					if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
					{
						component.Tag = buffer;
					}
					ImGui::PopItemWidth();
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
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Projection");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					auto& camera = component.Camera;

					const char* projTypesStrings[] = { "Perspective", "Orthographic" };
					const char* currentProjType = projTypesStrings[(s32)camera.GetProjectionType()];

					if (ImGui::BeginCombo("##Projection", currentProjType))
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
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
					{
						f32 fov = camera.GetPerspectiveFOV();
						f32 nearPlane = camera.GetPerspectiveNear();
						f32 farPlane = camera.GetPerspectiveFar();

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Perspective FOV");
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);
						if (ImGui::DragFloat("##PerspectiveFOV", &fov))
							camera.SetPerspectiveFOV(fov);
						ImGui::PopItemWidth();
						ImGui::Columns(1);

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Perspective Near");
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);
						if (ImGui::DragFloat("##PerspectiveNear", &nearPlane))
							camera.SetPerspectiveNear(nearPlane);
						ImGui::PopItemWidth();
						ImGui::Columns(1);

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Perspective Far");
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);
						if (ImGui::DragFloat("##PerspectiveFar", &farPlane))
							camera.SetPerspectiveFar(farPlane);
						ImGui::PopItemWidth();
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
						ImGui::PushItemWidth(-1);
						if (ImGui::DragFloat("##OrthographicSize", &orthoSize))
							camera.SetOrthographicSize(orthoSize);
						ImGui::PopItemWidth();
						ImGui::Columns(1);

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Orthographic Near");
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);
						if (ImGui::DragFloat("##OrthographicNear", &orthoNear))
							camera.SetOrthographicNear(orthoNear);
						ImGui::PopItemWidth();
						ImGui::Columns(1);

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text("Orthographic Far");
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);
						if (ImGui::DragFloat("##OrthographicFar", &orthoFar))
							camera.SetOrthographicFar(orthoFar);
						ImGui::PopItemWidth();
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
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Mesh");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::InputText("##Mesh", component.Mesh ? (char*)component.Mesh->GetAssetFilepath().stem().string().c_str() : "None", 50, ImGuiInputTextFlags_ReadOnly);
					ImGui::PopItemWidth();
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
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Environment");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::InputText("##EnvironmentMap", component.EnvironmentMap ? (char*)component.EnvironmentMap->GetAssetFilepath().stem().string().c_str() : "None", 50, ImGuiInputTextFlags_ReadOnly);
					ImGui::PopItemWidth();
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
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Color");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::ColorEdit3("##Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_DisplayRGB);
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Intensity");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.1f, 5.0f, "%.2f");
					ImGui::PopItemWidth();
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
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Color");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::ColorEdit3("##Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_DisplayRGB);
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Intensity");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.1f, 5.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Attenuation");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat3("##Attenuation", glm::value_ptr(component.AttenuationFactors), 0.05f, 0.0f, 10.0f, "%.2f");
					ImGui::PopItemWidth();
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
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Color");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::ColorEdit3("##Color", glm::value_ptr(component.Color), ImGuiColorEditFlags_DisplayRGB);
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Direction");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat3("##Direction", glm::value_ptr(component.Direction), 0.05f, -1.0f, 1.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Cone Angle");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat("##ConeAngle", &component.ConeAngle, 0.05f, 0.0f, 45.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Intensity");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat("##Intensity", &component.Intensity, 0.05f, 0.1f, 5.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Attenuation");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat3("##Attenuation", glm::value_ptr(component.AttenuationFactors), 0.05f, 0.0f, 10.0f, "%.2f");
					ImGui::PopItemWidth();
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

			if (m_Entity.HasComponent<ScriptComponent>())
			{
				Utils::DrawComponent<ScriptComponent>("Script", m_Entity, true, [entity = m_Entity](auto& component)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Class");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					// Get all class names
					const auto& scriptClassMap = ScriptEngine::GetScriptClasses();

					HashSet<String> scriptClassNames;
					scriptClassNames.reserve(scriptClassMap.size());

					for (auto& [clsName, cls] : scriptClassMap)
						scriptClassNames.insert(clsName);

					auto it = scriptClassNames.find(component.ScriptClass);
					const char* currentClass = it != scriptClassNames.end() ? (*it).c_str() : "";

					if (ImGui::BeginCombo("##ScriptClass", currentClass))
					{
						for (auto& scriptClassName : scriptClassNames)
						{
							bool isSelected = currentClass == scriptClassName.c_str();
							if (ImGui::Selectable(scriptClassName.c_str(), isSelected))
							{
								currentClass = scriptClassName.c_str();
								component.ScriptClass = currentClass;
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					if (Scene* scene = ScriptEngine::GetRunningScene())
					{
						// Get the fields from the script instance
						if (Ref<ScriptInstance> scriptInstance = ScriptEngine::GetScriptInstance(entity))
						{
							const ScriptVariableMap& classVariableMap = scriptInstance->GetClass()->GetMemberVariables();

							for (auto& [name, variable] : classVariableMap)
							{
								ImGui::Columns(2);
								ImGui::SetColumnWidth(0, 150.0f);
								ImGui::Text(name.c_str());
								ImGui::NextColumn();
								ImGui::PushItemWidth(-1);

								ScriptVariableType varType = variable.GetType();
								String imguiID = fmt::format("##{}", name.c_str());

								if (varType == ScriptVariableType::Float)
								{
									f32 data = scriptInstance->GetMemberValue<f32>(name);
									if (ImGui::DragFloat(imguiID.c_str(), &data))
									{
										scriptInstance->SetMemberValue(name, data);
									}
								}
								else if (varType == ScriptVariableType::Int)
								{
									s32 data = scriptInstance->GetMemberValue<s32>(name);
									if (ImGui::DragInt(imguiID.c_str(), &data))
									{
										scriptInstance->SetMemberValue(name, data);
									}
								}
								else if (varType == ScriptVariableType::Bool)
								{
									bool data = scriptInstance->GetMemberValue<bool>(name);
									if (ImGui::Checkbox(imguiID.c_str(), &data))
									{
										scriptInstance->SetMemberValue(name, data);
									}
								}
								else if (varType == ScriptVariableType::Vec2)
								{
									glm::vec2& data = scriptInstance->GetMemberValue<glm::vec2>(name);
									if (ImGui::DragFloat2(imguiID.c_str(), glm::value_ptr(data)))
									{
										scriptInstance->SetMemberValue(name, data);
									}
								}
								else if (varType == ScriptVariableType::Vec3)
								{
									glm::vec3& data = scriptInstance->GetMemberValue<glm::vec3>(name);
									if (ImGui::DragFloat3(imguiID.c_str(), glm::value_ptr(data)))
									{
										scriptInstance->SetMemberValue(name, data);
									}
								}
								else if (varType == ScriptVariableType::Vec4)
								{
									glm::vec4& data = scriptInstance->GetMemberValue<glm::vec4>(name);
									if (ImGui::DragFloat4(imguiID.c_str(), glm::value_ptr(data)))
									{
										scriptInstance->SetMemberValue(name, data);
									}
								}
								else if (varType == ScriptVariableType::Entity)
								{
									ScriptWrappers::Entity& data = scriptInstance->GetMemberValue<ScriptWrappers::Entity>(name);
									String entityTag = data.IsValid() ? data.GetTag() : "";

									ImGui::InputText(imguiID.c_str(), entityTag.data(), entityTag.size(), ImGuiInputTextFlags_ReadOnly);

									if (ImGui::BeginDragDropTarget())
									{
										if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_SRC_ENTITY"))
										{
											Entity srcEntity = *(Entity*)payload->Data;
											scriptInstance->SetMemberValue(name, ScriptWrappers::Entity(srcEntity.GetUUID()));
										}

										ImGui::EndDragDropTarget();
									}
								}

								ImGui::PopItemWidth();
								ImGui::Columns(1);
							}
						}
					}
					else
					{
						// Get the fields from the script field map for the entity
						if (Ref<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(component.ScriptClass))
						{
							const ScriptVariableMap& classVariableMap = scriptClass->GetMemberVariables();
							ScriptVariableMap& scriptVarMap = ScriptEngine::GetScriptVariableMap(entity);

							for (auto& [name, variable] : classVariableMap)
							{
								ImGui::Columns(2);
								ImGui::SetColumnWidth(0, 150.0f);
								ImGui::Text(name.c_str());
								ImGui::NextColumn();
								ImGui::PushItemWidth(-1);

								if (scriptVarMap.find(name) == scriptVarMap.end())
								{
									// The field has not been set yet
									scriptVarMap.emplace(name, variable);
								}

								ScriptVariable& var = scriptVarMap[name];
								ScriptVariableType varType = var.GetType();
								String imguiID = fmt::format("##{}", name.c_str());

								if (varType == ScriptVariableType::Float)
								{
									f32 data = var.GetValue<f32>();
									if (ImGui::DragFloat(imguiID.c_str(), &data))
									{
										var.SetValue(data);
									}
								}
								else if (varType == ScriptVariableType::Int)
								{
									s32 data = var.GetValue<s32>();
									if (ImGui::DragInt(imguiID.c_str(), &data))
									{
										var.SetValue(data);
									}
								}
								else if (varType == ScriptVariableType::Bool)
								{
									bool data = var.GetValue<bool>();
									if (ImGui::Checkbox(imguiID.c_str(), &data))
									{
										var.SetValue(data);
									}
								}
								else if (varType == ScriptVariableType::Vec2)
								{
									glm::vec2& data = var.GetValue<glm::vec2>();
									if (ImGui::DragFloat2(imguiID.c_str(), glm::value_ptr(data)))
									{
										var.SetValue(data);
									}
								}
								else if (varType == ScriptVariableType::Vec3)
								{
									glm::vec3& data = var.GetValue<glm::vec3>();
									if (ImGui::DragFloat3(imguiID.c_str(), glm::value_ptr(data)))
									{
										var.SetValue(data);
									}
								}
								else if (varType == ScriptVariableType::Vec4)
								{
									glm::vec4& data = var.GetValue<glm::vec4>();
									if (ImGui::DragFloat4(imguiID.c_str(), glm::value_ptr(data)))
									{
										var.SetValue(data);
									}
								}
								else if (varType == ScriptVariableType::Entity)
								{
									// We don't have a running scene set in the ScriptEngine at the moment so we can't use the ScriptWrappers::Entity directly to get the entity name
									// so we would have to get the active scene from the SceneHierarchyPanel
									Ref<Scene> activeScene = EditorLayer::Get().GetSceneHierarchyPanel().GetScene();
									Entity entity = activeScene->FindEntityByUUID(var.GetValue<ScriptWrappers::Entity>().GetUUID());
									String entityTag = entity ? entity.GetTag() : "";

									ImGui::InputText(imguiID.c_str(), entityTag.data(), entityTag.size(), ImGuiInputTextFlags_ReadOnly);

									if (ImGui::BeginDragDropTarget())
									{
										if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_SRC_ENTITY"))
										{
											Entity srcEntity = *(Entity*)payload->Data;
											var.SetValue(ScriptWrappers::Entity(srcEntity.GetUUID()));
										}

										ImGui::EndDragDropTarget();
									}
								}

								ImGui::PopItemWidth();
								ImGui::Columns(1);
							}
						}
					}

				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Script";
				data.ComponentHash = entt::type_id<ScriptComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<ScriptComponent>(); };
			}

			if (m_Entity.HasComponent<RigidbodyComponent>())
			{
				Utils::DrawComponent<RigidbodyComponent>("Rigidbody", m_Entity, true, [](auto& component)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Type");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					const char* bodyTypeStrings[] = { "Static", "Dynamic" };
					const char* currentBodyType = bodyTypeStrings[(s32)component.Type];

					if (ImGui::BeginCombo("##Type", currentBodyType))
					{
						for (u32 i = 0; i < _countof(bodyTypeStrings); i++)
						{
							bool isSelected = currentBodyType == bodyTypeStrings[i];
							if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
							{
								currentBodyType = bodyTypeStrings[i];
								component.Type = (RigidbodyComponent::RigidbodyType)i;
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Mass");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat("##Mass", &component.Mass, 0.05f, 0.1f, 0.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Fixed Rotation");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::Checkbox("X", &component.FixedRotation[0]);
					ImGui::SameLine();
					ImGui::Checkbox("Y", &component.FixedRotation[1]);
					ImGui::SameLine();
					ImGui::Checkbox("Z", &component.FixedRotation[2]);
					ImGui::PopItemWidth();
					ImGui::Columns(1);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Rigidbody";
				data.ComponentHash = entt::type_id<RigidbodyComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<RigidbodyComponent>(); };
			}

			if (m_Entity.HasComponent<BoxColliderComponent>())
			{
				Utils::DrawComponent<BoxColliderComponent>("Box Collider", m_Entity, true, [](auto& component)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Center");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat3("##Center", glm::value_ptr(component.Center), 0.05f, 0.0f, 0.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Size");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat3("##Size", glm::value_ptr(component.Size), 0.05f, 0.0f, 0.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Restitution");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat("##Restitution", &component.Restitution, 0.05f, 0.0f, 1.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Static Friction");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat("##StaticFriction", &component.StaticFriction, 0.05f, 0.0f, 0.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);

					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Dynamic Friction");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::DragFloat("##DynamicFriction", &component.DynamicFriction, 0.05f, 0.0f, 0.0f, "%.2f");
					ImGui::PopItemWidth();
					ImGui::Columns(1);
				});
			}
			else
			{
				ComponentData& data = missingComponents.emplace_back();
				data.ComponentName = "Box Collider";
				data.ComponentHash = entt::type_id<BoxColliderComponent>().hash();
				data.AddComponentFn = [](Entity entity) { entity.AddComponent<BoxColliderComponent>(); };
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