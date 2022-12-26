#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Tools/ContentTools.h"
#include "FileDialog.h"

#include <imgui.h>

namespace Atom
{
    template<typename ImportSettingsType>
    class ImportDialog
    {
    public:
        virtual void Open();
        virtual void OnImGuiRender() = 0;
    protected:
        template<typename SettingsUIFn, typename ImportAssetFn>
        void DrawUI(const char* title, const char* fileFilter, ImportAssetFn importFn, SettingsUIFn settingsFn);
    protected:
        bool                          m_IsOpened = false;
        Vector<std::filesystem::path> m_SelectedFiles;
        ImportSettingsType            m_ImportSettings;
    };

    class TextureImportDialog : public ImportDialog<TextureImportSettings>
    {
    public:
        virtual void OnImGuiRender() override;
    };

    class MeshImportDialog : public ImportDialog<MeshImportSettings>
    {
    public:
        virtual void OnImGuiRender() override;
    };

    // -----------------------------------------------------------------------------------------------------------------------------
    template<typename ImportSettingsType>
    inline void ImportDialog<ImportSettingsType>::Open()
    {
        m_IsOpened = true;
        m_SelectedFiles.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<typename ImportSettingsType>
    template<typename SettingsUIFn, typename ImportAssetFn>
    inline void ImportDialog<ImportSettingsType>::DrawUI(const char* title, const char* fileFilter, ImportAssetFn importFn, SettingsUIFn settingsFn)
    {
        if (m_IsOpened)
        {
            ImGui::OpenPopup(title);

            // Always center this window when appearing
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_Always);

            if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_NoResize))
            {
                ImGui::Text("Source files");
                if (ImGui::BeginTable("##SourceFileTable", 1, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY, ImVec2{ 0.0f, 180.0f }))
                {
                    for (auto& path : m_SelectedFiles)
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
                    std::filesystem::path path = FileDialog::OpenFile(fileFilter);

                    if (!path.empty())
                        m_SelectedFiles.push_back(path);
                }

                // ---------------------------------------------------------------------------------------------------------------
                ImGui::Text("Settings");
                if (ImGui::BeginTable("##TextureSettingsTable", 2, ImGuiTableFlags_BordersOuter))
                {
                    settingsFn();
                    ImGui::EndTable();
                }

                // -----------------------------------------------------------------------------------------------------------------

                f32 importButtonWidth = (ImGui::GetWindowContentRegionMax().x - ImGui::GetStyle().WindowPadding.x * 2.0f) / 2.0f;
                if (ImGui::Button("Import", ImVec2(importButtonWidth, 32)))
                {
                    for (auto& path : m_SelectedFiles)
                        importFn(path);

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