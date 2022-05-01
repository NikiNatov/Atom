#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/KeyCodes.h"
#include "Atom/Core/MouseCodes.h"

#include "AtomWin.h"
#include <glm\glm.hpp>

namespace Atom
{
	class Input
	{
	public:
		static void	Initialize(HWND windowHandle);
		static bool IsKeyPressed(Key keycode);
		static bool IsMouseButtonPressed(MouseButton button);

		static glm::vec2 GetMousePosition();
		static void SetMouseCursor(bool enabled);
		static void SetMousePosition(const glm::vec2& position);
		static bool IsCursorEnabled();
	private:
		static HWND ms_WindowHandle;
		static bool	ms_CursorEnabled;
	};
}