#include "atompch.h"
#include "ImGuiWrapper.h"

#include "Atom/Renderer/Renderer.h"

#include <imgui.h>

namespace Atom
{
    namespace ScriptWrappers
    {
        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::SameLine()
        {
            ImGui::SameLine();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec2 GUI::GetWindowSize()
        {
            ImVec2 size = ImGui::GetWindowSize();
            return { size.x, size.y };
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec2 GUI::GetWindowPosition()
        {
            ImVec2 pos = ImGui::GetWindowPos();
            return { pos.x, pos.y };
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool GUI::BeginDragDropTarget()
        {
            return ImGui::BeginDragDropTarget();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::EndDragDropTarget()
        {
            return ImGui::EndDragDropTarget();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool GUI::BeginDragDropSource()
        {
            return ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::EndDragDropSource()
        {
            ImGui::EndDragDropSource();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::SetDragDropPayload(const String& name, const pybind11::object& data)
        {
            ImGui::SetDragDropPayload(name.c_str(), &data, sizeof(pybind11::object));
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        pybind11::object GUI::GetDragDropPayload(const String& name)
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(name.c_str()))
                return *(pybind11::object*)payload->Data;

            return pybind11::none();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::Text(const String& text)
        {
            ImGui::Text(text.c_str());
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::Text(const String& text, const glm::vec2& offset)
        {
            ImGui::SetCursorPos(ImVec2{ offset.x, offset.y });
            ImGui::Text(text.c_str());
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool GUI::Button(const String& name, const glm::vec2& size)
        {
            return ImGui::Button(name.c_str(), ImVec2{ size.x, size.y });
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool GUI::Button(const String& name, const glm::vec2& size, const glm::vec2& offset)
        {
            ImGui::SetCursorPos(ImVec2{ offset.x, offset.y });
            return ImGui::Button(name.c_str(), ImVec2{ size.x, size.y });
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool GUI::ImageButton(ScriptWrappers::Texture2D image, const glm::vec2& size, const glm::vec2& uv0, const glm::vec2& uv1)
        {
            return ImGui::ImageButton(image.GetTexture() ? (ImTextureID)image.GetTexture().get() : (ImTextureID)Renderer::GetErrorTexture().get(), ImVec2{size.x, size.y}, ImVec2{ uv0.x, uv0.y }, ImVec2{ uv1.x, uv1.y });
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool GUI::ImageButton(ScriptWrappers::Texture2D image, const glm::vec2& size, const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& offset)
        {
            ImGui::SetCursorPos(ImVec2{ offset.x, offset.y });
            return ImGui::ImageButton(image.GetTexture() ? (ImTextureID)image.GetTexture().get() : (ImTextureID)Renderer::GetErrorTexture().get(), ImVec2{ size.x, size.y }, ImVec2{ uv0.x, uv0.y }, ImVec2{ uv1.x, uv1.y });
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::Image(ScriptWrappers::Texture2D image, const glm::vec2& size, const glm::vec2& uv0, const glm::vec2& uv1)
        {
            return ImGui::Image(image.GetTexture() ? (ImTextureID)image.GetTexture().get() : (ImTextureID)Renderer::GetErrorTexture().get(), ImVec2{ size.x, size.y }, ImVec2{ uv0.x, uv0.y }, ImVec2{ uv1.x, uv1.y });
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::Image(ScriptWrappers::Texture2D image, const glm::vec2& size, const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& offset)
        {
            ImGui::SetCursorPos(ImVec2{ offset.x, offset.y });
            return ImGui::Image(image.GetTexture() ? (ImTextureID)image.GetTexture().get() : (ImTextureID)Renderer::GetErrorTexture().get(), ImVec2{ size.x, size.y }, ImVec2{ uv0.x, uv0.y }, ImVec2{ uv1.x, uv1.y });
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::BeginChildWindow(const String& title, const glm::vec2& size, const glm::vec2& offset)
        {
            ImGui::SetCursorPos(ImVec2{ offset.x, offset.y });
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
            ImGui::BeginChild(title.c_str(), ImVec2{ size.x, size.y }, true, ImGuiWindowFlags_NoResize);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void GUI::EndChildWindow()
        {
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
    }
}
