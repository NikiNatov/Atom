#include "atompch.h"

#include "Window.h"
#include <imgui.h>
#include <backends/imgui_impl_win32.h>

namespace Atom
{
	EXTERN_C IMAGE_DOS_HEADER __ImageBase;
	static HINSTANCE s_hInstance;

	// -----------------------------------------------------------------------------------------------------------------------------
	Window::Window(const WindowProperties& properties)
		: m_Title(properties.Title),
		m_Width(properties.Width),
		m_Height(properties.Height),
		m_VSync(properties.VSync),
		m_EventCallback(properties.EventCallback),
		m_Minimized(false),
		m_NeedsResize(false)
	{
		s_hInstance = (HINSTANCE)&__ImageBase;

		// Initialize window
		m_WindowClass = {};
		m_WindowClass.cbSize = sizeof(m_WindowClass);
		m_WindowClass.style = CS_OWNDC;
		m_WindowClass.lpfnWndProc = WindowProcSetup;
		m_WindowClass.cbClsExtra = 0;
		m_WindowClass.cbWndExtra = 0;
		m_WindowClass.hInstance = s_hInstance;
		m_WindowClass.hCursor = nullptr;
		m_WindowClass.hbrBackground = nullptr;
		m_WindowClass.lpszMenuName = nullptr;
		m_WindowClass.lpszClassName = "Atom Engine Window";
		RegisterClassEx(&m_WindowClass);

		m_WindowRect.left = 100;
		m_WindowRect.right = m_Width + m_WindowRect.left;
		m_WindowRect.top = 100;
		m_WindowRect.bottom = m_Height + m_WindowRect.top;

		ATOM_ENGINE_ASSERT(AdjustWindowRect(&m_WindowRect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_SIZEBOX, FALSE), "Failed to adjust window rect!");

		m_WindowHandle = CreateWindowEx(0, m_WindowClass.lpszClassName, m_Title.c_str(), WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_MAXIMIZEBOX | WS_SIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, m_WindowRect.right - m_WindowRect.left, m_WindowRect.bottom - m_WindowRect.top, nullptr, nullptr, s_hInstance, this);

		ATOM_ENGINE_ASSERT(m_WindowHandle, "Failed to create the window!");

		// Create rendering context
		m_GfxDevice = CreateScope<Device>(GPUPreference::HighPerformance, "Main Device");
		m_SwapChain = CreateScope<SwapChain>(m_WindowHandle, m_Width, m_Height);

		ShowWindow(m_WindowHandle, SW_MAXIMIZE);
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	Window::~Window()
	{
		UnregisterClass(m_WindowClass.lpszClassName, s_hInstance);
		DestroyWindow(m_WindowHandle);

		// Make sure the swapchain is destroyed before the device
		m_SwapChain.reset();
		m_GfxDevice.reset();
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	void Window::ProcessEvents()
	{
		// Process window messages
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	void Window::SwapBuffers()
	{
		m_SwapChain->Present(m_VSync);
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	void Window::SetEventCallback(const EventCallbackFn& callback)
	{
		m_EventCallback = callback;
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	void Window::SetMinimized(bool state)
	{
		m_Minimized = state;
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	void Window::ToggleVSync()
	{
		m_VSync = !m_VSync;
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	LRESULT WINAPI Window::WindowProcSetup(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		if (Msg == WM_CREATE)
		{
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* const window = static_cast<Window*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::WindowProc));
			return WindowProc(hWnd, Msg, wParam, lParam);
		}

		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	LRESULT WINAPI Window::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
			return true;

		switch (Msg)
		{
			case WM_DESTROY:
			case WM_CLOSE:
			{
				WindowClosedEvent e;
				window->m_EventCallback(e);
				PostQuitMessage(0);
				return 0;
			}
			case WM_KILLFOCUS:
			{
				break;
			}
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				KeyPressedEvent e(static_cast<Key>(wParam), 0);
				window->m_EventCallback(e);
				break;
			}
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				KeyReleasedEvent e(static_cast<Key>(wParam));
				window->m_EventCallback(e);
				break;
			}
			case WM_CHAR:
			{
				KeyTypedEvent e(static_cast<Key>(wParam));
				window->m_EventCallback(e);
				break;
			}
			case WM_MOUSEMOVE:
			{
				const POINTS pt = MAKEPOINTS(lParam);

				MouseMovedEvent e(pt.x, pt.y);
				window->m_EventCallback(e);
				break;
			}
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			{
				MouseButtonPressedEvent e(static_cast<MouseButton>(wParam));
				window->m_EventCallback(e);
				break;
			}
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			{
				MouseButtonReleasedEvent e(static_cast<MouseButton>(wParam));
				window->m_EventCallback(e);
				break;
			}
			case WM_MOUSEWHEEL:
			{
				const POINTS pt = MAKEPOINTS(lParam);
				const int delta = GET_WHEEL_DELTA_WPARAM(wParam);

				MouseScrolledEvent e(0, delta);
				window->m_EventCallback(e);
				break;
			}
			case WM_SIZE:
			{
				window->m_NeedsResize = wParam != SIZE_MINIMIZED;
				window->m_Width = LOWORD(lParam);
				window->m_Height = HIWORD(lParam);

				if (wParam == SIZE_MINIMIZED)
					window->SetMinimized(true);
				else if (wParam == SIZE_RESTORED)
					window->SetMinimized(false);

				break;
			}
		}

		// Make sure we resize the swap chain only when the mouse button is not pressed
		if (window->m_NeedsResize && GetAsyncKeyState(VK_LBUTTON) >= 0)
		{
			window->m_SwapChain->Resize(window->m_Width, window->m_Height);

			WindowResizedEvent e(window->m_Width, window->m_Height);
			window->m_EventCallback(e);
			window->m_NeedsResize = false;
		}

		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
}
