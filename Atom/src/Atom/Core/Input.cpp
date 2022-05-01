#include "atompch.h"

#include "Atom/Core/Core.h"
#include "Atom/Core/Input.h"

namespace Atom
{
	HWND Input::ms_WindowHandle = 0;
	bool Input::ms_CursorEnabled = true;

	// --------------------------------------------------------------------------------------------------------------------------------------
	void Input::Initialize(HWND windowHandle)
	{
		ms_WindowHandle = windowHandle;
	}

	// --------------------------------------------------------------------------------------------------------------------------------------
	bool Input::IsKeyPressed(Key key)
	{
		if (GetAsyncKeyState((s32)key) & (1 << 15))
			return true;

		return false;
	}

	// --------------------------------------------------------------------------------------------------------------------------------------
	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		if (GetAsyncKeyState((s32)button) & (1 << 15))
			return true;

		return false;
	}

	// --------------------------------------------------------------------------------------------------------------------------------------
	glm::vec2 Input::GetMousePosition()
	{
		POINT point;

		bool result = GetCursorPos(&point);
		ATOM_ENGINE_ASSERT(result, "Failed to get the cursor position!");

		result = ScreenToClient(ms_WindowHandle, &point);
		ATOM_ENGINE_ASSERT(result, "Could not convert from screen coordinates to client coordinates!");
		return { point.x, point.y };
	}

	// --------------------------------------------------------------------------------------------------------------------------------------
	void Input::SetMouseCursor(bool enabled)
	{
		ms_CursorEnabled = enabled;

		if (enabled)
		{
			while (ShowCursor(true) < 0);
		}
		else
		{
			while (ShowCursor(false) >= 0);
		}
	}

	// --------------------------------------------------------------------------------------------------------------------------------------
	void Input::SetMousePosition(const glm::vec2& position)
	{
		POINT point = { (LONG)position.x, (LONG)position.y };
		ClientToScreen(ms_WindowHandle, &point);
		SetCursorPos(point.x, point.y);
	}

	// --------------------------------------------------------------------------------------------------------------------------------------
	bool Input::IsCursorEnabled()
	{
		return ms_CursorEnabled;
	}
}
