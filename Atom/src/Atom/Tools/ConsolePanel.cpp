#include "atompch.h"
#include "ConsolePanel.h"

#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Device.h"
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
    void ConsolePanel::Initialize()
    {
        Image2D infoImage("resources/icons/info_icon.png");
        Image2D warningImage("resources/icons/warning_icon.png");
        Image2D errorImage("resources/icons/error_icon.png");

        TextureDescription desc;
        desc.Format = TextureFormat::RGBA8;

        desc.Width = infoImage.GetWidth();
        desc.Height = infoImage.GetHeight();
        ms_InfoIcon = CreateRef<Texture2D>(desc, "InfoIcon");

        desc.Width = warningImage.GetWidth();
        desc.Height = warningImage.GetHeight();
        ms_WarningIcon = CreateRef<Texture2D>(desc, "WarningIcon");

        desc.Width = errorImage.GetWidth();
        desc.Height = errorImage.GetHeight();
        ms_ErrorIcon = CreateRef<Texture2D>(desc, "ErrorIcon");

        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = CreateRef<CommandBuffer>(CommandQueueType::Copy, "IconsCopyCmdBuffer");
        copyCommandBuffer->Begin();
        copyCommandBuffer->UploadTextureData(infoImage.GetPixelData().data(), ms_InfoIcon.get());
        copyCommandBuffer->UploadTextureData(warningImage.GetPixelData().data(), ms_WarningIcon.get());
        copyCommandBuffer->UploadTextureData(errorImage.GetPixelData().data(), ms_ErrorIcon.get());
        copyCommandBuffer->End();
        copyQueue->ExecuteCommandList(copyCommandBuffer.get());
        copyQueue->Flush();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ConsolePanel::Shutdown()
    {
        ms_InfoIcon = nullptr;
        ms_WarningIcon = nullptr;
        ms_ErrorIcon = nullptr;
    }

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
        Utils::ImageToggleButton("Infos", (ImTextureID)ms_InfoIcon.get(), ImVec2{ toggleButtonSize, toggleButtonSize }, displayInfo);
        ImGui::SameLine();
        Utils::ImageToggleButton("Warnings", (ImTextureID)ms_WarningIcon.get(), ImVec2{ toggleButtonSize, toggleButtonSize }, displayWarnings);
        ImGui::SameLine();
        Utils::ImageToggleButton("Errors", (ImTextureID)ms_ErrorIcon.get(), ImVec2{ toggleButtonSize, toggleButtonSize }, displayErrors);

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
                            currentIcon = ms_InfoIcon.get();
                            break;
                        }
                        case ConsoleMessage::Severity::Warning:
                        {
                            color = ImColor(255, 200, 0);
                            currentIcon = ms_WarningIcon.get();
                            break;
                        }
                        case ConsoleMessage::Severity::Error:
                        {
                            color = ImColor(255, 100, 100);
                            currentIcon = ms_ErrorIcon.get();
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