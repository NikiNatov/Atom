#include "AssetPanel.h"

#include "../EditorResources.h"
#include "Atom/Asset/AssetManager.h"

#include <imgui.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    AssetPanel::AssetPanel()
        : m_AssetsDirectory(AssetManager::GetAssetsFolder()), m_CurrentDirectory(m_AssetsDirectory)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetPanel::OnImGuiRender()
    {
		ImGui::Begin("Assets");

		if (std::filesystem::is_directory(m_CurrentDirectory))
		{
			if (m_CurrentDirectory != m_AssetsDirectory)
			{
				if (ImGui::Button("<-"))
				{
					m_CurrentDirectory = m_CurrentDirectory.parent_path();
				}
			}

			ImGuiStyle& style = ImGui::GetStyle();
			f32 window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			ImVec2 button_sz = { 128.0f, 128.0f };

			for (auto it : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				const auto& path = it.path();
				auto relativePath = std::filesystem::relative(path, m_AssetsDirectory);
				std::filesystem::path filename = relativePath.filename();

				ImGui::BeginGroup();

				if (it.is_directory())
				{
					ImGui::PushID(filename.string().c_str());
					ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
					ImGui::ImageButton((ImTextureID)EditorResources::FolderIcon.get(), button_sz, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0, { 0.0f, 0.0f, 0.0f, 0.0f });
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						m_CurrentDirectory /= path.filename();
					}
					ImGui::TextWrapped(filename.string().c_str());
					ImGui::PopStyleColor();
					ImGui::PopID();
				}
				else
				{
					ImGui::PushID(filename.string().c_str());
					ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });

					if (filename.extension() == ".atmtex2d")
					{
						ImGui::ImageButton((ImTextureID)EditorResources::Texture2DAssetIcon.get(), button_sz, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0, { 0.0f, 0.0f, 0.0f, 0.0f });

						if (ImGui::BeginDragDropSource())
						{
							ImGui::Text("%s", filename.string().c_str());
							UUID assetUUID = AssetManager::GetUUIDForAssetPath(relativePath);
							ImGui::SetDragDropPayload("DRAG_TEXTURE2D", &assetUUID, sizeof(UUID));
							ImGui::EndDragDropSource();
						}

						ImGui::TextWrapped(filename.string().c_str());
					}
					else if (filename.extension() == ".atmtexcube")
					{
						ImGui::ImageButton((ImTextureID)EditorResources::TextureCubeAssetIcon.get(), button_sz, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0, { 0.0f, 0.0f, 0.0f, 0.0f });

						if (ImGui::BeginDragDropSource())
						{
							ImGui::Text("%s", filename.string().c_str());
							UUID assetUUID = AssetManager::GetUUIDForAssetPath(relativePath);
							ImGui::SetDragDropPayload("DRAG_TEXTURE_CUBE", &assetUUID, sizeof(UUID));
							ImGui::EndDragDropSource();
						}

						ImGui::TextWrapped(filename.string().c_str());
					}
					else if (filename.extension() == ".atmmesh")
					{
						ImGui::ImageButton((ImTextureID)EditorResources::MeshAssetIcon.get(), button_sz, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0, { 0.0f, 0.0f, 0.0f, 0.0f });

						if (ImGui::BeginDragDropSource())
						{
							ImGui::Text("%s", filename.string().c_str());
							UUID assetUUID = AssetManager::GetUUIDForAssetPath(relativePath);
							ImGui::SetDragDropPayload("DRAG_MESH", &assetUUID, sizeof(UUID));
							ImGui::EndDragDropSource();
						}

						ImGui::TextWrapped(filename.string().c_str());
					}
					else if (filename.extension() == ".atmmat")
					{
						ImGui::ImageButton((ImTextureID)EditorResources::MaterialAssetIcon.get(), button_sz, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0, { 0.0f, 0.0f, 0.0f, 0.0f });

						if (ImGui::BeginDragDropSource())
						{
							ImGui::Text("%s", filename.string().c_str());
							UUID assetUUID = AssetManager::GetUUIDForAssetPath(relativePath);
							ImGui::SetDragDropPayload("DRAG_MATERIAL", &assetUUID, sizeof(UUID));
							ImGui::EndDragDropSource();
						}

						ImGui::TextWrapped(filename.string().c_str());
					}
					else if (filename.extension() == ".atmscene")
					{
						ImGui::ImageButton((ImTextureID)EditorResources::SceneAssetIcon.get(), button_sz, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0, { 0.0f, 0.0f, 0.0f, 0.0f });

						if (ImGui::BeginDragDropSource())
						{
							ImGui::Text("%s", filename.string().c_str());
							UUID assetUUID = AssetManager::GetUUIDForAssetPath(relativePath);
							ImGui::SetDragDropPayload("DRAG_SCENE", &assetUUID, sizeof(UUID));
							ImGui::EndDragDropSource();
						}

						ImGui::TextWrapped(filename.string().c_str());
					}

					ImGui::PopStyleColor();
					ImGui::PopID();
				}

				ImGui::EndGroup();

				f32 last_button_x2 = ImGui::GetItemRectMax().x;
				f32 next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
				if (next_button_x2 < window_visible_x2)
					ImGui::SameLine(0, 20);
			}
		}

		ImGui::End();
    }
}
