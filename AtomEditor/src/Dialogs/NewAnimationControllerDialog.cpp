#include "NewAnimationControllerDialog.h"
#include "FileDialog.h"

#include "Atom/Tools/ContentTools.h"
#include "Atom/Asset/AssetManager.h"

#include <imgui.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void NewAnimationControllerDialog::Open()
    {
        m_IsOpened = true;
        m_ControllerName = "NewAnimationController";
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void NewAnimationControllerDialog::OnImGuiRender()
    {
        if (m_IsOpened)
        {
            ImGui::OpenPopup("New Animation Controller");

            // Always center this window when appearing
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_Always);

            if (ImGui::BeginPopupModal("New Animation Controller", nullptr, ImGuiWindowFlags_NoResize))
            {
                // ---------------------------------------------------------------------------------------------------------------
                ImGui::Text("Settings");
                if (ImGui::BeginTable("##AnimationControllerSettingsTable", 2, ImGuiTableFlags_BordersOuter))
                {
                    // Name 
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Controller name");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushItemWidth(-1);

                        char buffer[30];
                        memset(buffer, 0, sizeof(buffer));
                        strcpy_s(buffer, sizeof(buffer), m_ControllerName.c_str());

                        if (ImGui::InputText("##ControllerName", buffer, sizeof(buffer)))
                            m_ControllerName = buffer;

                        ImGui::PopItemWidth();
                        ImGui::Columns(1);
                    }

                    // Initial state index
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Initial state");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::PushItemWidth(-1);
                        ImGui::InputScalar("##InitialState", ImGuiDataType_U16, &m_InitialStateIdx);
                        ImGui::PopItemWidth();
                        ImGui::Columns(1);
                    }

                    ImGui::EndTable();
                }

                // ---------------------------------------------------------------------------------------------------------------
                ImGui::Text("Animations");
                if (ImGui::BeginTable("##AnimationsTable", 1, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY, ImVec2{ 0.0f, 180.0f }))
                {
                    for (auto& path : m_AnimationPaths)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", path.string().c_str());
                    }

                    ImGui::EndTable();
                }

                f32 browseButtonWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetStyle().WindowPadding.x;
                if (ImGui::Button("Browse Files", ImVec2(browseButtonWidth, 32.0f)))
                {
                    const char* fileFilter = "Aton Animation Files (*.atmanim)\0*.atmanim\0";
                    std::filesystem::path path = FileDialog::OpenFile(fileFilter);

                    if (!path.empty())
                        m_AnimationPaths.push_back(path);
                }

                // ---------------------------------------------------------------------------------------------------------------
                f32 createButtonWidth = (ImGui::GetWindowContentRegionMax().x - ImGui::GetStyle().WindowPadding.x * 2.0f) / 2.0f;
                if (ImGui::Button("Create", ImVec2(createButtonWidth, 32)))
                {
                    if (!m_ControllerName.empty() && !m_AnimationPaths.empty() && m_InitialStateIdx < m_AnimationPaths.size())
                    {
                        Vector<Ref<Animation>> animationAssets;
                        animationAssets.reserve(m_AnimationPaths.size());

                        for (auto& path : m_AnimationPaths)
                        {
                            UUID animUUID = AssetManager::GetUUIDForAssetPath(path);
                            animationAssets.push_back(AssetManager::GetAsset<Animation>(animUUID, true));
                        }

                        ContentTools::CreateAnimationControllerAsset(animationAssets, m_InitialStateIdx, std::filesystem::path("AnimationControllers") / (m_ControllerName + ".atmanimcontroller"));
                    }

                    ImGui::CloseCurrentPopup();
                    m_IsOpened = false;
                }

                ImGui::SameLine();

                if (ImGui::Button("Close", ImVec2(createButtonWidth, 32)))
                {
                    ImGui::CloseCurrentPopup();
                    m_IsOpened = false;
                }

                ImGui::EndPopup();
            }
        }
    }
}

