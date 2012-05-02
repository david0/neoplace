
#include <windows.h>
#include <tchar.h>
#include "trayicon.h"
#include "resources.h"

#define WM_ICON_CLICK (WM_USER+1)
#define LOG_ERROR(text) MessageBox(NULL, TEXT(text), TEXT("Error"), MB_ICONERROR | MB_OK);
#define WND_CLASS " toolbar"

enum {
	ID_LAST
};

NOTIFYICONDATA trayicon_data = { };

HMENU popup_menu;

callback_functionPtr *functionptr_array = NULL;
unsigned item_count = 1;

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
			   0, CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, HWND_MESSAGE, NULL,
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

	popup_menu = CreatePopupMenu();

	functionptr_array = malloc(sizeof(callback_functionPtr));
	functionptr_array[0] = NULL;
}

void trayicon_remove()
{
	Shell_NotifyIcon(NIM_DELETE, &trayicon_data);
	free(functionptr_array);
}

void trayicon_add_item(char *text, callback_functionPtr functionPtr)
{
	if (!text) {
		functionptr_array[0] = functionPtr;
	} else {
	  item_count++;
		functionptr_array =
		    realloc(functionptr_array, sizeof(callback_functionPtr) * item_count-1);
		AppendMenu(popup_menu, MF_STRING, ID_LAST + item_count-1, text);
		functionptr_array[item_count-1] = functionPtr;
	}

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
			switch (lParam) {
			case WM_LBUTTONUP:
				{
					if (functionptr_array[0])
						(*functionptr_array[0]) ();
					break;
				}
			case WM_RBUTTONUP:
				{
					POINT cursor_position;
					GetCursorPos(&cursor_position);
					TrackPopupMenu(popup_menu, 0, cursor_position.x,
						       cursor_position.y, 0, hWnd, 0);
					break;

				}
			}
			break;
		}
	case WM_COMMAND:
		{
			//find out if a menu item was clicked
			if (lParam >= ID_LAST && lParam < ID_LAST + item_count)
				(*functionptr_array[lParam - ID_LAST+1]) ();
			break;
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
