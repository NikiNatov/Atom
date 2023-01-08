#include "NewProjectDialog.h"

#include "../EditorLayer.h"
#include "Atom/Project/Project.h"
#include "Atom/Asset/AssetManager.h"
#include "Atom/Tools/ContentTools.h"

#include <imgui.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void NewProjectDialog::Open()
    {
        m_IsOpened = true;
        m_ProjectName = "UntitledProject";
        m_StartSceneName = "UntitledScene";
        m_ProjectLocation = std::filesystem::canonical(std::filesystem::current_path());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void NewProjectDialog::OnImGuiRender()
    {
        if (m_IsOpened)
        {
            ImGui::OpenPopup("New project");

            // Always center this window when appearing
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_Always);

            if (ImGui::BeginPopupModal("New project", nullptr, ImGuiWindowFlags_NoResize))
            {
                ImGui::Text("Settings");
                if (ImGui::BeginTable("##TextureSettingsTable", 2, ImGuiTableFlags_BordersOuter))
                {
                    // Name 
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Project Name");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushItemWidth(-1);

                        char buffer[30];
                        memset(buffer, 0, sizeof(buffer));
                        strcpy_s(buffer, sizeof(buffer), m_ProjectName.c_str());

                        if (ImGui::InputText("##ProjectName", buffer, sizeof(buffer)))
                            m_ProjectName = buffer;

                        ImGui::PopItemWidth();
                        ImGui::Columns(1);
                    }

                    // Start Scene Name 
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Start scene name");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushItemWidth(-1);

                        char buffer[30];
                        memset(buffer, 0, sizeof(buffer));
                        strcpy_s(buffer, sizeof(buffer), m_StartSceneName.c_str());

                        if (ImGui::InputText("##StartSceneName", buffer, sizeof(buffer)))
                            m_StartSceneName = buffer;

                        ImGui::PopItemWidth();
                        ImGui::Columns(1);
                    }

                    // Location 
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Location");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushItemWidth(-1);

                        char buffer[256];
                        memset(buffer, 0, sizeof(buffer));
                        strcpy_s(buffer, sizeof(buffer), m_ProjectLocation.string().c_str());

                        if (ImGui::InputText("##ProjectLocation", buffer, sizeof(buffer)))
                            m_ProjectLocation = buffer;

                        ImGui::PopItemWidth();
                        ImGui::Columns(1);
                    }
                    ImGui::EndTable();
                }

                f32 importButtonWidth = (ImGui::GetWindowContentRegionMax().x - ImGui::GetStyle().WindowPadding.x * 2.0f) / 2.0f;
                if (ImGui::Button("Create", ImVec2(importButtonWidth, 32)))
                {
                    if (!m_ProjectName.empty() && std::filesystem::is_directory(m_ProjectLocation))
                    {
                        EditorLayer::Get().NewProject(m_ProjectName, m_StartSceneName, m_ProjectLocation);
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
