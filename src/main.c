/*
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * neoplace - a tiling extension for windows inspired by gtile
 * 
 * David Otto, 2012-04-24
 */

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "resources.h"
#include "trayicon.h"

enum {
	ID_BUTTON_RESIZE,
	ID_LAST
};

#define LOG_ERROR(text) MessageBox(NULL, TEXT(text), TEXT("Error"), MB_ICONERROR | MB_OK);
#define APPNAME "neoplace"
#define BUTTON_SIZE 15
#define BUTTON_SPACE 3
#define BUTTON_MAX 10
#define MARGIN 10
#define WINDOW_STYLE   WS_BORDER | WS_CAPTION | WS_SYSMENU
#define WINDOW_EXSTYLE WS_EX_TOOLWINDOW

unsigned BUTTON_ROWS = 6;
unsigned BUTTON_COLS = 6;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
void exitApplication();
void toggleWindowVisible();
HWND createMatrixButton(HWND parent, HINSTANCE hInstance, unsigned i, unsigned j);
HWND createMainWindow(HINSTANCE current_instance);
void centerOnWindow(HWND windowToCenter, HWND windowToCenterOn);
void uncheckAllButtons();
HWND getTopWindow();
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND top_window, main_window;
bool is_visible = false;

HWND buttons[BUTTON_MAX][BUTTON_MAX];
bool is_checking = false;	//<is currently checking the size matrix ?
POINT last_click_position;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	top_window = GetForegroundWindow();

	trayicon_init(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON)), APPNAME);
	trayicon_add_item(NULL, &toggleWindowVisible);
	trayicon_add_item("Exit", &exitApplication);

	main_window = createMainWindow(hInstance);
	SetTimer(main_window, 1, 500, NULL);

	is_visible = false;

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

void toggleWindowVisible()
{
	is_visible = !is_visible;
	if (is_visible) {
		top_window = getTopWindow();
		centerOnWindow(main_window, top_window);
		ShowWindow(main_window, SW_SHOWDEFAULT);
		UpdateWindow(main_window);
		SetFocus(main_window);
		SetTimer(main_window, 1, 500, NULL);
	} else {
		ShowWindow(main_window, SW_HIDE);
		is_checking = false;
		KillTimer(main_window, 1);
	}

}

POINT getTopMidOfWindow(HWND window)
{
	POINT result = { };
	RECT rect;
	POINT cli;

	GetWindowRect(window, &rect);

	cli.y = 0;
	ClientToScreen(window, &cli);

	result.x = rect.left + (rect.right - rect.left) / 2;
	result.y = cli.y;

	return result;
}

void centerOnWindow(HWND windowToCenter, HWND windowToCenterOn)
{
	POINT point;
	RECT rect = { };
	LONG width, height;

	if (!GetWindowRect(windowToCenter, &rect)) {
		LOG_ERROR("Could not get window dimensions!");
		return;
	}
	point = getTopMidOfWindow(windowToCenterOn);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	point.x = point.x - width / 2;
	point.y = point.y;

	SetWindowPos(windowToCenter, HWND_TOPMOST, point.x, point.y, 0, 0, SWP_NOSIZE);

	is_checking = false;
	uncheckAllButtons();
}

void resizeMainWindow(HWND win)
{
	RECT winsize;

	winsize.left = 0;
	winsize.top = 0;
	winsize.right = BUTTON_COLS * (BUTTON_SIZE + BUTTON_SPACE) + 2 * MARGIN;
	winsize.bottom = BUTTON_ROWS * (BUTTON_SIZE + BUTTON_SPACE) + 2 * MARGIN;

	AdjustWindowRectEx(&winsize, WINDOW_STYLE, false, WINDOW_EXSTYLE);

	SetWindowPos(win, 0, 0, 0, winsize.right - winsize.left,
		     winsize.bottom - winsize.top, SWP_NOMOVE | SWP_NOZORDER);
}

HWND createMainWindow(HINSTANCE current_instance)
{
	WNDCLASSEX wc = { };
	LPCTSTR MainWndClass = TEXT(APPNAME);
	HWND window_handle;

	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = &MainWndProc;
	wc.hInstance = current_instance;
	wc.style = 0;
	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;
	wc.hCursor = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.lpszClassName = MainWndClass;
#ifdef WITH_ICON
	wc.hIcon =
	    (HICON) LoadImage(current_instance,
			      MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, LR_SHARED);
	wc.hIconSm =
	    (HICON) LoadImage(current_instance, MAKEINTRESOURCE(IDI_APPICON),
			      IMAGE_ICON, 16, 16, LR_SHARED);
#endif

	if (!RegisterClassEx(&wc)) {
		LOG_ERROR("Error registering window class.");
		return NULL;
	}
	window_handle =
	    CreateWindowEx(WINDOW_EXSTYLE, MainWndClass, TEXT(APPNAME),
			   WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, 100,
			   300, NULL, NULL, current_instance, NULL);

	if (!window_handle) {
		LOG_ERROR("Error creating main window.");
		return NULL;
	}
	return window_handle;
}

void exitApplication()
{
	trayicon_remove();
	PostQuitMessage(0);
}

HWND createMatrixButton(HWND parent, HINSTANCE hInstance, unsigned i, unsigned j)
{
	static HWND button;
	button = CreateWindow("button",
			      "",
			      WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX |
			      BS_PUSHLIKE | BS_FLAT,
			      MARGIN + j * (BUTTON_SIZE + BUTTON_SPACE),
			      MARGIN + i * (BUTTON_SIZE + BUTTON_SPACE),
			      BUTTON_SIZE, BUTTON_SIZE, parent, NULL, hInstance, NULL);
	return button;
}

void initGUI(HWND win, HINSTANCE hInstance)
{
	unsigned i, j;

	/*
	 * CreateWindow("button", "Resize!",
	 * WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 0,200,150,50, win, NULL,
	 * hInstance, ID_BUTTON_nESIZE);
	 */
	for (i = 0; i < BUTTON_ROWS; i++)
		for (j = 0; j < BUTTON_COLS; j++)
			buttons[i][j] = createMatrixButton(win, hInstance, i, j);

}

void uncheckAllButtons()
{
	unsigned x, y;
	for (x = 0; x < BUTTON_COLS; x++)
		for (y = 0; y < BUTTON_ROWS; y++)
			SendMessage(buttons[y][x], BM_SETCHECK, BST_UNCHECKED, 0);
}

HWND getTopWindow()
{
	static HWND taskbar, desktop;
	HWND window;
	if (!taskbar)
		taskbar = FindWindow("Shell_TrayWnd", NULL);
	if (!desktop)
		desktop = GetDesktopWindow();

	window = GetForegroundWindow();
	while ( ((GetWindowLongPtr(window, GWL_STYLE) & (WS_SIZEBOX | WS_VISIBLE)) != (WS_SIZEBOX | WS_VISIBLE)) ||
		   window == taskbar || window == desktop || window == main_window
	       || window == NULL) {

		window = GetNextWindow(window, GW_HWNDNEXT);

	}
	return window;
}

void resizeAndMoveWindowTo(HWND win, RECT area)
{
	RECT workarea, rect;
	HMONITOR current_monitor;
	MONITORINFO monitor_info;

#if (_WIN32_WINNT >= 0x0500 || _WIN32_WINDOWS >= 0x0410)
	current_monitor = MonitorFromWindow(main_window, MONITOR_DEFAULTTONEAREST);
	monitor_info.cbSize = sizeof(monitor_info);
	GetMonitorInfo(current_monitor, &monitor_info);

	workarea = monitor_info.rcWork;
#else
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);
#endif

	rect.left =
	    workarea.left + (double)(area.left) * (double)(workarea.right -
							   workarea.left) / BUTTON_COLS;
	rect.right =
	    workarea.left + (double)(1 + area.right) * (double)(workarea.right -
								workarea.left) / BUTTON_COLS;
	rect.top =
	    workarea.top + (double)(area.top) * (double)(workarea.bottom -
							 workarea.top) / BUTTON_ROWS;
	rect.bottom =
	    workarea.top + (double)(1 +
				    area.bottom) * (double)(workarea.bottom -
							    workarea.top) / BUTTON_ROWS;

	ShowWindowAsync(win, SW_SHOWNORMAL);

	SetWindowPos(win, 0, rect.left, rect.top, rect.right - rect.left,
		     rect.bottom - rect.top, 0);

	uncheckAllButtons();
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HINSTANCE hInstance;
	switch (msg) {
	case WM_CREATE:
		{
			hInstance = ((LPCREATESTRUCT) lParam)->hInstance;
			initGUI(hWnd, hInstance);
			resizeMainWindow(hWnd);
			centerOnWindow(hWnd, top_window);
			return 0;
		}

	case WM_COMMAND:
		{
			unsigned x, y;

			for (x = 0; x < BUTTON_COLS; x++)
				for (y = 0; y < BUTTON_ROWS; y++) {
					if (lParam == (LPARAM) buttons[y][x]) {
						if (is_checking) {
							RECT area;

							area.top = min(y, last_click_position.y);
							area.bottom = max(y, last_click_position.y);

							area.left = min(x, last_click_position.x);
							area.right = max(x, last_click_position.x);

							resizeAndMoveWindowTo(top_window, area);
							centerOnWindow(main_window, top_window);
						}
						last_click_position.x = x;
						last_click_position.y = y;
					}
				}

			is_checking = !is_checking;
			return 0;
		}
		break;

		switch (LOWORD(wParam)) {
		case ID_BUTTON_RESIZE:
			if (HIWORD(wParam) == BN_CLICKED) {
				SetWindowPos(top_window, 0, 0, 0, 100, 100, 0);
			}
			return 0;

		}
		break;

	case WM_TIMER:
		{
			HWND foreground = getTopWindow();
			if (foreground != top_window) {
				top_window = foreground;
				if (is_visible) {
					centerOnWindow(hWnd, top_window);
					SetFocus(hWnd);
				}
			}
			SetTimer(hWnd, 1, 500, NULL);
			return 0;
		}

	case WM_KILLFOCUS:
		{
			if (is_visible) {
				SetTimer(hWnd, 1, 200, NULL);
				is_checking = false;
			}
		}
		break;

	case WM_CLOSE:
		{
			toggleWindowVisible();
			return 0;
		}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
