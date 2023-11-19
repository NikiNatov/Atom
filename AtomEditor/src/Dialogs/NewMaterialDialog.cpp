#include "NewMaterialDialog.h"

#include "Atom/Tools/ContentTools.h"
#include "Atom/Renderer/ShaderLibrary.h"

#include <imgui.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void NewMaterialDialog::Open()
    {
        m_IsOpened = true;
        m_MaterialName = "NewMaterial";
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void NewMaterialDialog::OnImGuiRender()
    {
        if (m_IsOpened)
        {
            ImGui::OpenPopup("New Material");

            // Always center this window when appearing
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_Always);

            if (ImGui::BeginPopupModal("New Material", nullptr, ImGuiWindowFlags_NoResize))
            {
                ImGui::Text("Settings");
                if (ImGui::BeginTable("##MaterialSettingsTable", 2, ImGuiTableFlags_BordersOuter))
                {
                    // Name 
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Material Name");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushItemWidth(-1);

                        char buffer[30];
                        memset(buffer, 0, sizeof(buffer));
                        strcpy_s(buffer, sizeof(buffer), m_MaterialName.c_str());

                        if (ImGui::InputText("##MaterialName", buffer, sizeof(buffer)))
                            m_MaterialName = buffer;

                        ImGui::PopItemWidth();
                        ImGui::Columns(1);
                    }

                    // Shader
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Shader");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushItemWidth(-1);

                        char buffer[30];
                        memset(buffer, 0, sizeof(buffer));
                        strcpy_s(buffer, sizeof(buffer), m_ShaderName.c_str());

                        if (ImGui::InputText("##Shader", buffer, sizeof(buffer)))
                            m_ShaderName = buffer;

                        ImGui::PopItemWidth();
                        ImGui::Columns(1);
                    }

                    ImGui::EndTable();
                }

                f32 importButtonWidth = (ImGui::GetWindowContentRegionMax().x - ImGui::GetStyle().WindowPadding.x * 2.0f) / 2.0f;
                if (ImGui::Button("Create", ImVec2(importButtonWidth, 32)))
                {
                    if (!m_MaterialName.empty() && ShaderLibrary::Get().Exists(m_ShaderName))
                    {
                        ContentTools::CreateMaterialAsset(m_ShaderName, std::filesystem::path("Materials") / (m_MaterialName + ".atmmat"));
                    }

                    ImGui::CloseCurrentPopup();
                    m_IsOpened = false;
                }

                ImGui::SameLine();

                if (ImGui::Button("Close", ImVec2(importButtonWidth, 32)))
                {
                    ImGui::CloseCurrentPopup();
                    m_IsOpened = false;
                }

                ImGui::EndPopup();
            }
        }
    }
}

