#pragma once
#include "Core.h"

#include "AtomWin.h"

namespace Atom
{
	enum class MouseButton : s32
	{
		Left = VK_LBUTTON,
		Right = VK_RBUTTON,
		Middle = VK_MBUTTON,
		X1 = VK_XBUTTON1,
		X2 = VK_XBUTTON2,
	};
}