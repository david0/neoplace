
#include <windows.h>
#include <tchar.h>
#include "trayicon.h"
#include "resources.h"

#define WM_ICON_CLICK (WM_USER+1)
#define LOG_ERROR(text) MessageBox(NULL, TEXT(text), TEXT("Error"), MB_ICONERROR | MB_OK);
#define WND_CLASS " toolbar"

NOTIFYICONDATA trayicon_data = { };

LRESULT CALLBACK trayicon_messageloop(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool trayicon_init(HICON icon, char tooltip[])
{
	HWND hidden_window;
	WNDCLASSEX wc = { 0 };
	TCHAR class_name[256];

	// build new class name
	_tcscpy(class_name, tooltip);
	_tcscat(class_name, WND_CLASS);

	wc.cbSize = sizeof(wc);
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.style = 0;
	wc.lpfnWndProc = &trayicon_messageloop;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = class_name;
	if (!RegisterClassEx(&wc)) {
		LOG_ERROR("Error registering window class.");
		return false;
	}
	hidden_window =
	    CreateWindowEx(0, class_name, class_name,
			   0, CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, NULL, NULL,
			   GetModuleHandle(NULL), NULL);

	if (!hidden_window) {
		LOG_ERROR("Error creating hidden window.");
		return false;
	}

	trayicon_data.cbSize = sizeof(trayicon_data);
	trayicon_data.hIcon = icon;
	trayicon_data.hWnd = hidden_window;
	trayicon_data.uCallbackMessage = WM_ICON_CLICK;
	trayicon_data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	strncpy(trayicon_data.szTip, tooltip, 64);
	trayicon_data.szTip[64] = '\0';
	Shell_NotifyIcon(NIM_ADD, &trayicon_data);
}

void trayicon_remove()
{
	Shell_NotifyIcon(NIM_DELETE, &trayicon_data);
}

void trayicon_add_item(char *text, void (functionPtr) ())
{
	//FIXME: must be implemented
}

LRESULT CALLBACK trayicon_messageloop(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
		{
			break;
		}
	case WM_ICON_CLICK:
		{
			break;
		}

	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
