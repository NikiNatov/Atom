#include "atompch.h"
#include "ImportDialog.h"

#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureImportDialog::OnImGuiRender()
    {
        DrawUI("Texture Import Settings", "Texture Files (*.png, *.hdr)\0*.png;*.hdr\0", 
            [this](auto path) { ContentTools::ImportTextureAsset(path, AssetManager::GetAssetsFolder() / "Textures", m_ImportSettings); }, [this]()
        {
            // Texture type 
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Type");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);

                const char* textureTypeStr[] = { "Texture2D", "Cubemap" };
                const char* currentType = textureTypeStr[(s32)m_ImportSettings.Type - 1];

                if (ImGui::BeginCombo("##TextureType", currentType))
                {
                    for (u32 i = 0; i < _countof(textureTypeStr); i++)
                    {
                        bool isSelected = currentType == textureTypeStr[i];
                        if (ImGui::Selectable(textureTypeStr[i], isSelected))
                        {
                            currentType = textureTypeStr[i];
                            m_ImportSettings.Type = (TextureType)(i + 1);
                        }

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                ImGui::PopItemWidth();
                ImGui::Columns(1); 

            }

            // Cubemap size
            if (m_ImportSettings.Type == TextureType::TextureCube)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Cubemap size");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);

                const char* cubemapSizeStr[] = { "32", "64", "128", "256", "512", "1024", "2048", "4096" };
                const char* currentSize = cubemapSizeStr[(s32)glm::log2((f32)m_ImportSettings.CubemapSize) - 5];

                if (ImGui::BeginCombo("##CubemapSize", currentSize))
                {
                    for (u32 i = 0; i < _countof(cubemapSizeStr); i++)
                    {
                        bool isSelected = currentSize == cubemapSizeStr[i];
                        if (ImGui::Selectable(cubemapSizeStr[i], isSelected))
                        {
                            currentSize = cubemapSizeStr[i];
                            m_ImportSettings.CubemapSize = (u32)glm::pow(2, i + 5);
                        }

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }
            }
         
            // Format 
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Format");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);

                const char* formatStr[] = { "R8", "RGBA8", "RG16 Float", "RGBA16 Float", "RG32 Float", "RGBA32 Float" };
                const char* currentFormat = formatStr[(s32)m_ImportSettings.Format - 1];

                if (ImGui::BeginCombo("##Format", currentFormat))
                {
                    for (u32 i = 0; i < _countof(formatStr); i++)
                    {
                        bool isSelected = currentFormat == formatStr[i];
                        if (ImGui::Selectable(formatStr[i], isSelected))
                        {
                            currentFormat = formatStr[i];
                            m_ImportSettings.Format = (TextureFormat)(i + 1);
                        }

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                ImGui::PopItemWidth();
                ImGui::Columns(1);
            }

            // Filter 
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Filter");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);

                const char* filterStr[] = { "Linear", "Nearest", "Anisotropic" };
                const char* currentFilter = filterStr[(s32)m_ImportSettings.Filter - 1];

                if (ImGui::BeginCombo("##Filter", currentFilter))
                {
                    for (u32 i = 0; i < _countof(filterStr); i++)
                    {
                        bool isSelected = currentFilter == filterStr[i];
                        if (ImGui::Selectable(filterStr[i], isSelected))
                        {
                            currentFilter = filterStr[i];
                            m_ImportSettings.Filter = (TextureFilter)(i + 1);
                        }

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                ImGui::PopItemWidth();
                ImGui::Columns(1);
            }

            // Wrap mode 
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Wrap mode");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);

                const char* wrapStr[] = { "Clamp", "Repeat" };
                const char* currentWrapMode = wrapStr[(s32)m_ImportSettings.Wrap - 1];

                if (ImGui::BeginCombo("##Wrap", currentWrapMode))
                {
                    for (u32 i = 0; i < _countof(wrapStr); i++)
                    {
                        bool isSelected = currentWrapMode == wrapStr[i];
                        if (ImGui::Selectable(wrapStr[i], isSelected))
                        {
                            currentWrapMode = wrapStr[i];
                            m_ImportSettings.Wrap = (TextureWrap)(i + 1);
                        }

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                ImGui::PopItemWidth();
                ImGui::Columns(1);
            }

            // IsReadable 
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Readable");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                ImGui::Checkbox("##Readable", &m_ImportSettings.IsReadable);
                ImGui::PopItemWidth();
                ImGui::Columns(1);
            }
        });
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void MeshImportDialog::OnImGuiRender()
    {
        DrawUI("Mesh Import Settings", "FBX Files (*.fbx, *.FBX)\0*.fbx;*.FBX\0", 
            [this](auto path) { ContentTools::ImportMeshAsset(path, AssetManager::GetAssetsFolder() / "Meshes", m_ImportSettings); }, [this]()
        {
            // IsReadable 
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Readable");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                ImGui::Checkbox("##Readable", &m_ImportSettings.IsReadable);
                ImGui::PopItemWidth();
                ImGui::Columns(1);
            }

            // SmoothNormals 
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Smooth normals");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                ImGui::Checkbox("##SmoothNormals", &m_ImportSettings.SmoothNormals);
                ImGui::PopItemWidth();
                ImGui::Columns(1);
            }

            // PreserveHierarchy 
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Preserve hierarchy");
                ImGui::TableSetColumnIndex(1);
                ImGui::PushItemWidth(-1);
                ImGui::Checkbox("##PreserveHierarchy", &m_ImportSettings.PreserveHierarchy);
                ImGui::PopItemWidth();
                ImGui::Columns(1);
            }
        });
    }
}