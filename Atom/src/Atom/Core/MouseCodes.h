#include "Atom\Core\Core.h"

#if defined(ATOM_PLATFORM_WINDOWS)
#include <Windows.h>
#endif // ATOM_PLATFORM_WINDOWS

namespace Atom
{
	enum class MouseButton : s32
	{
#if defined(ATOM_PLATFORM_WINDOWS)

		Left = VK_LBUTTON,
		Right = VK_RBUTTON,
		Middle = VK_MBUTTON,
		X1 = VK_XBUTTON1,
		X2 = VK_XBUTTON2,

#endif // ATOM_PLATFORM_WINDOWS

	};
}