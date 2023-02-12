#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Scripting/ScriptWrappers/Renderer/TextureWrapper.h"
#include <glm/glm.hpp>
#include <pybind11/pybind11.h>

namespace Atom
{
    namespace ScriptWrappers
    {
        class GUI
        {
        public:
            static void SameLine();
            static glm::vec2 GetWindowSize();
            static glm::vec2 GetWindowPosition();
            static bool BeginDragDropTarget();
            static void EndDragDropTarget();
            static bool BeginDragDropSource();
            static void EndDragDropSource();
            static void SetDragDropPayload(const String& name, const pybind11::object& data);
            static pybind11::object GetDragDropPayload(const String& name);
            static void Text(const String& text);
            static void Text(const String& text, const glm::vec2& offset);
            static bool Button(const String& name, const glm::vec2& size);
            static bool Button(const String& name, const glm::vec2& size, const glm::vec2& offset);
            static bool ImageButton(ScriptWrappers::Texture2D image, const glm::vec2& size, const glm::vec2& uv0, const glm::vec2& uv1);
            static bool ImageButton(ScriptWrappers::Texture2D image, const glm::vec2& size, const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& offset);
            static void Image(ScriptWrappers::Texture2D image, const glm::vec2& size, const glm::vec2& uv0, const glm::vec2& uv1);
            static void Image(ScriptWrappers::Texture2D image, const glm::vec2& size, const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& offset);
            static void BeginChildWindow(const String& title, const glm::vec2& size, const glm::vec2& offset);
            static void EndChildWindow();

        };
    }
}