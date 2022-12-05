#include "atompch.h"
#include "ConsolePanel.h"
#include "../EditorResources.h"

#include <imgui.h>

namespace Atom
{
    namespace Utils
    {
        void ImageToggleButton(const char* strID, ImTextureID textureID, const ImVec2& size, bool& var)
        {
            static constexpr ImVec4 activeColor = ImVec4{ 1.0, 1.0, 1.0, 1.0 };
            static constexpr ImVec4 inactiveColor = ImVec4{ 0.4, 0.4, 0.4, 1.0 };

            if (ImGui::InvisibleButton(strID, size))
            {
                var = !var;
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - size.x - ImGui::GetStyle().ItemSpacing.x);
            ImGui::Image(textureID, size, ImVec2{ 0.0f, 0.0f }, ImVec2{ 1.0f, 1.0f }, var ? activeColor : inactiveColor);
        }
    }

    Vector<ConsoleMessage> ConsolePanel::ms_Messages;

    // -----------------------------------------------------------------------------------------------------------------------------
    void ConsolePanel::AddMessage(const ConsoleMessage& message)
    {
        ms_Messages.push_back(message);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ConsolePanel::OnImGuiRender()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 5.0f });
        ImGui::Begin("Console");

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5.0f);
        if (ImGui::Button("Clear"))
            ms_Messages.clear();

        ImGui::SameLine();

        static bool displayInfo = true;
        static bool displayWarnings = true;
        static bool displayErrors = true;

        f32 iconSize = 28.0f;
        f32 toggleButtonSize = 24.0f;
        f32 rowPadding = 8.0f;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - toggleButtonSize * 3 - ImGui::GetStyle().ItemSpacing.x * 3);
        Utils::ImageToggleButton("Infos", (ImTextureID)EditorResources::InfoIcon.get(), ImVec2{ toggleButtonSize, toggleButtonSize }, displayInfo);
        ImGui::SameLine();
        Utils::ImageToggleButton("Warnings", (ImTextureID)EditorResources::WarningIcon.get(), ImVec2{ toggleButtonSize, toggleButtonSize }, displayWarnings);
        ImGui::SameLine();
        Utils::ImageToggleButton("Errors", (ImTextureID)EditorResources::ErrorIcon.get(), ImVec2{ toggleButtonSize, toggleButtonSize }, displayErrors);

        ImGui::Separator();

        ImGui::BeginChild("ConsoleMessages");

        ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg;

        if (ImGui::BeginTable("MessageTable", 2, tableFlags))
        {
            ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed, iconSize + rowPadding * 2.0f);
            ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);

            for (const auto& msg : ms_Messages)
            {
                bool shouldDisplayMsg = (msg.GetSeverity() == ConsoleMessage::Severity::Info && displayInfo) ||
                                        (msg.GetSeverity() == ConsoleMessage::Severity::Warning && displayWarnings) ||
                                        (msg.GetSeverity() == ConsoleMessage::Severity::Error && displayErrors);

                if (shouldDisplayMsg)
                {
                    f32 textHeight = ImGui::CalcTextSize(msg.GetMessageString().c_str()).y + ImGui::GetStyle().FramePadding.y * 2.0f;

                    ImColor color(0xFFFFFFFF);
                    Texture2D* currentIcon = nullptr;

                    switch (msg.GetSeverity())
                    {
                        case ConsoleMessage::Severity::Info:
                        {
                            color = ImColor(70, 170, 255);
                            currentIcon = EditorResources::InfoIcon.get();
                            break;
                        }
                        case ConsoleMessage::Severity::Warning:
                        {
                            color = ImColor(255, 200, 0);
                            currentIcon = EditorResources::WarningIcon.get();
                            break;
                        }
                        case ConsoleMessage::Severity::Error:
                        {
                            color = ImColor(255, 100, 100);
                            currentIcon = EditorResources::ErrorIcon.get();
                            break;
                        }
                    }

                    ImGui::TableNextRow(0, iconSize);

                    // Icon
                    ImGui::TableSetColumnIndex(0);
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + rowPadding);
                    ImGui::Image((ImTextureID)currentIcon, ImVec2{ iconSize, iconSize });

                    // Message
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + iconSize / 2.0f - textHeight / 2.0f);
                    ImGui::PushStyleColor(ImGuiCol_Text, color.Value);
                    ImGui::TextWrapped(msg.GetMessageString().c_str());
                    ImGui::PopStyleColor();
                }
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleVar();
    }
}