#include "pch.h"

BOOL IsMainWindow(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !IsMainWindow(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}

HWND FindMainWindowHandle(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(EnumWindowsCallback, (LPARAM)&data);
	return data.window_handle;
}

LONG_PTR originalWindowProc = NULL; //instantiate

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // WndProc wrapper
{ // this will be called each time WndProc is called.
	switch (uMsg) // Each keystroke will PostMessage with uMsg down and wParam the key.
	{
	case WM_LBUTTONDOWN:
		Input::KeyPressed[VK_LBUTTON] = true;
		break;
	case WM_LBUTTONUP:
		Input::KeyPressed[VK_LBUTTON] = false;
		break;
	case WM_RBUTTONDOWN:
		Input::KeyPressed[VK_RBUTTON] = true;
		break;
	case WM_RBUTTONUP:
		Input::KeyPressed[VK_RBUTTON] = false;
		break;
	case WM_MBUTTONDOWN:
		Input::KeyPressed[VK_MBUTTON] = true;
		break;
	case WM_MBUTTONUP:
		Input::KeyPressed[VK_MBUTTON] = false;
		break;
	case WM_MOUSEMOVE:
		Input::MousePosition = { (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam) };
		MainMenu->OnMouseMove();
		break;
	case WM_KEYDOWN:
		Input::KeyPressed[wParam] = true;
		break;
	case WM_KEYUP:
		Input::KeyPressed[wParam] = false;
		break;
	default:
		break;
	}

	if ((uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || (uMsg >= WM_LBUTTONDOWN && uMsg <= WM_MBUTTONUP)))
	{
		if(uMsg == WM_KEYDOWN)
		{
			switch (wParam)
			{
			case VK_F7:
				oneHit->Toggled = !oneHit->Toggled;
				break;
			case VK_F8:
				flyMode->Toggled = !flyMode->Toggled;
				flyMode->Callback(flyMode->Toggled);
				break;
			case VK_F9:
				godMode->Toggled = !godMode->Toggled;
				godMode->Callback(godMode->Toggled);
				break;
			case VK_F10:
				magnet->Toggled = !magnet->Toggled;
				break;
			}
		}

		if (Input::KeyPressed[VK_HOME])
		{
			MainMenu->Visible = !MainMenu->Visible;
			MainMenu->OnVisibleChanged(MainMenu->Visible);
		}

		MainMenu->OnKeyPress(Input::KeyPressed);
	}

	if (MainMenu->Visible && uMsg >= WM_LBUTTONDOWN && uMsg <= WM_MBUTTONUP)
	{
		return 0;
	}

	return CallWindowProcW((WNDPROC)originalWindowProc, hWnd, uMsg, wParam, lParam); // Call the original Function
}

namespace Input
{
	FVector2D MousePosition = {};
	bool KeyPressed[256] = {};

	void Initialize()
	{
		auto window = FindMainWindowHandle(GetCurrentProcessId());

		originalWindowProc = SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
	}
}

