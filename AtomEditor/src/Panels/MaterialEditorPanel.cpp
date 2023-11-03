#include "MaterialEditorPanel.h"

#include "Atom/Asset/AssetManager.h"
#include "Atom/Asset/AssetSerializer.h"
#include "Atom/Renderer/EngineResources.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialEditorPanel::OnImGuiRender()
    {
		ImGui::Begin("Material Editor");
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_MATERIAL"))
			{
				m_Material = AssetManager::GetAsset<MaterialAsset>(*(UUID*)payload->Data, true);
			}

			ImGui::EndDragDropTarget();
		}

		if (m_Material)
		{
			ImGui::Text("Material: ");
			ImGui::SameLine();
			ImGui::Text(m_Material->GetAssetFilepath().stem().string().c_str());

			ImGui::Text("Shader: ");
			ImGui::SameLine();
			ImGui::Text(m_Material->GetShader()->GetName().c_str());

			if (ImGui::CollapsingHeader("Textures", flags))
			{
				const auto& resourceTable = m_Material->GetShader()->GetShaderLayout().GetResourceDescriptorTable(ShaderBindPoint::Material);
				for (auto& resource : resourceTable.Resources)
				{
					if (resource.Type == ShaderResourceType::Texture2D)
					{
						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 150.0f);
						ImGui::Text(resource.Name.c_str());
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);

						Ref<TextureAsset> texture = m_Material->GetTexture(resource.Name.c_str());
						ImGui::ImageButton(texture ? (ImTextureID)texture->GetResource().get() : (ImTextureID)EngineResources::BlackTexture.get(), {64.0f, 64.0f}, {0.0f, 0.0f});

						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_TEXTURE2D"))
							{
								Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(*(UUID*)payload->Data, true);
								m_Material->SetTexture(resource.Name.c_str(), texture);

								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							ImGui::EndDragDropTarget();
						}

						ImGui::PopItemWidth();
						ImGui::Columns(1);

						ImGui::Separator();
					}
				}

			}

			if (ImGui::CollapsingHeader("Uniforms", flags))
			{
				const auto& constants = m_Material->GetShader()->GetShaderLayout().GetConstants(ShaderBindPoint::Material);
				for (auto& uniform : constants.Uniforms)
				{
					if (uniform.Name[0] == '_')
						continue;

					ImGui::PushID(uniform.Name.c_str());
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text(uniform.Name.c_str());
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					switch (uniform.Type)
					{
						case ShaderDataType::Int:
						{
							s32 value = m_Material->GetUniform<s32>(uniform.Name.c_str());

							if (ImGui::DragInt("", &value))
							{
								m_Material->SetUniform(uniform.Name.c_str(), value);
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
						case ShaderDataType::Int2:
						{
							glm::ivec2 value = m_Material->GetUniform<glm::ivec2>(uniform.Name.c_str());

							if (ImGui::DragInt2("", glm::value_ptr(value)))
							{
								m_Material->SetUniform(uniform.Name.c_str(), value);
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
						case ShaderDataType::Int3:
						{
							glm::ivec3 value = m_Material->GetUniform<glm::ivec3>(uniform.Name.c_str());

							if (ImGui::DragInt3("", glm::value_ptr(value)))
							{
								m_Material->SetUniform(uniform.Name.c_str(), value);
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
						case ShaderDataType::Int4:
						{
							glm::ivec4 value = m_Material->GetUniform<glm::ivec4>(uniform.Name.c_str());

							if (ImGui::DragInt4("", glm::value_ptr(value)))
							{
								m_Material->SetUniform(uniform.Name.c_str(), value);
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
						case ShaderDataType::Float:
						{
							f32 value = m_Material->GetUniform<f32>(uniform.Name.c_str());

							if (ImGui::DragFloat("", &value, 0.05f))
							{
								m_Material->SetUniform(uniform.Name.c_str(), value);
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
						case ShaderDataType::Float2:
						{
							glm::vec2 value = m_Material->GetUniform<glm::vec2>(uniform.Name.c_str());

							if (ImGui::DragFloat2("", glm::value_ptr(value), 0.05f))
							{
								m_Material->SetUniform(uniform.Name.c_str(), value);
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
						case ShaderDataType::Float3:
						{
							glm::vec3 value = m_Material->GetUniform<glm::vec3>(uniform.Name.c_str());

							if (uniform.Name.find("Color") != std::string::npos)
							{
								if (ImGui::ColorEdit3("", glm::value_ptr(value)))
								{
									m_Material->SetUniform(uniform.Name.c_str(), value);
									AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
								}
							}
							else
							{
								if (ImGui::DragFloat3("", glm::value_ptr(value), 0.05f))
								{
									m_Material->SetUniform(uniform.Name.c_str(), value);
									AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
								}
							}

							break;
						}
						case ShaderDataType::Float4:
						{
							glm::vec4 value = m_Material->GetUniform<glm::vec4>(uniform.Name.c_str());

							if (uniform.Name.find("Color") != std::string::npos)
							{
								if (ImGui::ColorEdit4("", glm::value_ptr(value)))
								{
									m_Material->SetUniform(uniform.Name.c_str(), value);
									AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
								}
							}
							else
							{
								if (ImGui::DragFloat4("", glm::value_ptr(value), 0.05f))
								{
									m_Material->SetUniform(uniform.Name.c_str(), value);
									AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
								}
							}

							break;
						}
						case ShaderDataType::Bool:
						{
							bool value = m_Material->GetUniform<bool>(uniform.Name.c_str());

							if (ImGui::Checkbox("", &value))
							{
								m_Material->SetUniform(uniform.Name.c_str(), value);
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
						case ShaderDataType::Mat2:
						{
							glm::mat2 value = glm::transpose(m_Material->GetUniform<glm::mat2>(uniform.Name.c_str()));
							bool valueChanged = false;

							valueChanged |= ImGui::DragFloat2("##Mat2x2Row1", glm::value_ptr(value[0]), 0.05f);
							valueChanged |= ImGui::DragFloat2("##Mat2x2Row2", glm::value_ptr(value[1]), 0.05f);

							if(valueChanged)
							{
								m_Material->SetUniform(uniform.Name.c_str(), glm::transpose(value));
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
						case ShaderDataType::Mat3:
						{
							glm::mat3 value = glm::transpose(m_Material->GetUniform<glm::mat3>(uniform.Name.c_str()));
							bool valueChanged = false;

							valueChanged |= ImGui::DragFloat3("##Mat3x3Row1", glm::value_ptr(value[0]), 0.05f);
							valueChanged |= ImGui::DragFloat3("##Mat3x3Row2", glm::value_ptr(value[1]), 0.05f);
							valueChanged |= ImGui::DragFloat3("##Mat3x3Row3", glm::value_ptr(value[2]), 0.05f);

							if (valueChanged)
							{
								m_Material->SetUniform(uniform.Name.c_str(), glm::transpose(value));
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
						case ShaderDataType::Mat4:
						{
							glm::mat4 value = glm::transpose(m_Material->GetUniform<glm::mat4>(uniform.Name.c_str()));
							bool valueChanged = false;

							valueChanged |= ImGui::DragFloat4("##Mat4x4Row1", glm::value_ptr(value[0]), 0.05f);
							valueChanged |= ImGui::DragFloat4("##Mat4x4Row2", glm::value_ptr(value[1]), 0.05f);
							valueChanged |= ImGui::DragFloat4("##Mat4x4Row3", glm::value_ptr(value[2]), 0.05f);
							valueChanged |= ImGui::DragFloat4("##Mat4x4Row4", glm::value_ptr(value[3]), 0.05f);

							if (valueChanged)
							{
								m_Material->SetUniform(uniform.Name.c_str(), glm::transpose(value));
								AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
							}

							break;
						}
					}

					ImGui::PopItemWidth();
					ImGui::Columns(1);
					ImGui::PopID();

					ImGui::Separator();
				}
			}

			if (ImGui::CollapsingHeader("Flags", flags))
			{
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Depth-tested");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					bool value = m_Material->GetFlag(MaterialFlags::DepthTested);

					if (ImGui::Checkbox("##DepthTested", &value))
					{
						m_Material->SetFlag(MaterialFlags::DepthTested, value);
						AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
					}

					ImGui::PopItemWidth();
					ImGui::Columns(1);
				}

				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Transparent");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					bool value = m_Material->GetFlag(MaterialFlags::Transparent);

					if (ImGui::Checkbox("##Transparent", &value))
					{
						m_Material->SetFlag(MaterialFlags::Transparent, value);
						AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
					}

					ImGui::PopItemWidth();
					ImGui::Columns(1);
				}

				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Two-sided");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					bool value = m_Material->GetFlag(MaterialFlags::TwoSided);

					if (ImGui::Checkbox("##TwoSided", &value))
					{
						m_Material->SetFlag(MaterialFlags::TwoSided, value);
						AssetSerializer::Serialize(m_Material->GetAssetFilepath(), m_Material);
					}

					ImGui::PopItemWidth();
					ImGui::Columns(1);
				}

				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 150.0f);
					ImGui::Text("Wireframe");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					bool value = m_Material->GetFlag(MaterialFlags::Wireframe);

					if (ImGui::Checkbox("##Wireframe", &value))
						m_Material->SetFlag(MaterialFlags::Wireframe, value);

					ImGui::PopItemWidth();
					ImGui::Columns(1);
				}
			}
		}
		ImGui::End();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialEditorPanel::SetMaterial(const Ref<MaterialAsset>& material)
    {
        m_Material = material;
    }
}

