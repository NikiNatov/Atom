#include "atompch.h"
#include "ConsolePanel.h"

#include <imgui.h>

namespace Atom
{
    Vector<ConsoleMessage> ConsolePanel::ms_Messages;

    // -----------------------------------------------------------------------------------------------------------------------------
    void ConsolePanel::AddMessage(const ConsoleMessage& message)
    {
        ms_Messages.push_back(message);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ConsolePanel::OnImGuiRender()
    {
        ImGui::Begin("Console");

        if (ImGui::Button("Clear"))
        {
            ms_Messages.clear();
        }

        ImGui::SameLine();

        static bool displayInfo = true;
        static bool displayWarnings = true;
        static bool displayErrors = true;

        ImGui::Checkbox("Infos", &displayInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warnings", &displayWarnings);
        ImGui::SameLine();
        ImGui::Checkbox("Errors", &displayErrors);

        ImGui::Separator();

        if (ImGui::BeginTable("MessageTable", 1, ImGuiTableFlags_RowBg))
        {
            for (const auto& msg : ms_Messages)
            {
                bool shouldDisplayMsg = (msg.GetSeverity() == ConsoleMessage::Severity::Info && displayInfo) ||
                    (msg.GetSeverity() == ConsoleMessage::Severity::Warning && displayWarnings) ||
                    (msg.GetSeverity() == ConsoleMessage::Severity::Error && displayErrors);

                if (shouldDisplayMsg)
                {
                    f32 rowHeight = 32.0f;
                    ImGui::TableNextRow(0, rowHeight);
                    ImGui::TableSetColumnIndex(0);

                    ImColor color(0xFFFFFFFF);
                    switch (msg.GetSeverity())
                    {
                        case ConsoleMessage::Severity::Info:
                        {
                            color = ImColor(70, 255, 70);
                            break;
                        }
                        case ConsoleMessage::Severity::Warning:
                        {
                            color = ImColor(255, 200, 0);
                            break;
                        }
                        case ConsoleMessage::Severity::Error:
                        {
                            color = ImColor(255, 100, 100);
                            break;
                        }
                    }

                    f32 textHeight = ImGui::CalcTextSize(msg.GetMessageString().c_str()).y + ImGui::GetStyle().FramePadding.y * 2.0f;
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + rowHeight / 2.0f - textHeight / 2.0f);
                    ImGui::TextColored(color, msg.GetMessageString().c_str());
                }
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }
}