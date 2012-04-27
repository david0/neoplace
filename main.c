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

/* neoplace - a tiling extension for windows
 *  inspired by gtile
 *
 *  David O., 2012-04-24
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
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND createMatrixButton(HWND parent, HINSTANCE hInstance, unsigned i, unsigned j);

HWND top_window, main_window;


HWND  buttons[BUTTON_MAX][BUTTON_MAX];
bool  is_checking = false;              //< is currently checking the size matrix?
POINT last_click_position;


void debug_window_title(HWND win) {
    char t[2000];
    GetWindowText(win, t, 2000);
    LOG_ERROR(t);
}

POINT getTopMidOfWindow(HWND window) {
    POINT result = {};
    RECT rect;
    POINT cli;

    GetWindowRect(window, &rect);

    cli.y = 0;
    ClientToScreen(window, &cli);

    result.x = rect.left + (rect.right - rect.left) / 2 ;
    result.y = cli.y;

    return result;
}


void centerOnWindow(HWND windowToCenter, HWND windowToCenterOn) {
    POINT point;
    RECT rect = {};
    LONG width, height;

    if(! GetWindowRect(windowToCenter, &rect)) {
        LOG_ERROR("Could not get window dimensions!");
        return;
    }


    point = getTopMidOfWindow(windowToCenterOn);

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    point.x  = max (0, point.x - width / 2);
    point.y = max(0, point.y);

    SetWindowPos(windowToCenter, HWND_TOPMOST, point.x, point.y, 0, 0 ,SWP_NOSIZE);
}

void resizeMainWindow (HWND win) {
    RECT winsize;

    winsize.left=0;
    winsize.top=0;
    winsize.right = BUTTON_COLS * (BUTTON_SIZE + BUTTON_SPACE) + 2 * MARGIN, BUTTON_ROWS * (BUTTON_SIZE + BUTTON_SPACE) + 2 * MARGIN;
    winsize.bottom = BUTTON_COLS * (BUTTON_SIZE + BUTTON_SPACE) + 2 * MARGIN, BUTTON_ROWS * (BUTTON_SIZE + BUTTON_SPACE) + 2 * MARGIN;

    AdjustWindowRectEx(&winsize, WINDOW_STYLE, false, WINDOW_EXSTYLE);

    SetWindowPos(win,0, 0, 0, winsize.right-winsize.left, winsize.bottom-winsize.top,  SWP_NOMOVE|SWP_NOZORDER);
}

HWND createMainWindow(HINSTANCE current_instance) {
    WNDCLASSEX wc = {};
    LPCTSTR MainWndClass = TEXT(APPNAME);
    HWND window_handle;

    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = &MainWndProc;
    wc.hInstance     = current_instance;
    wc.style = 0;
    wc.cbWndExtra = 0;
    wc.cbClsExtra =0 ;
    wc.hCursor       = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
    wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    wc.lpszClassName = MainWndClass;
#ifdef WITH_ICON
    wc.hIcon         = (HICON) LoadImage(current_instance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, LR_SHARED);
    wc.hIconSm       = (HICON) LoadImage(current_instance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_SHARED);
#endif

    if (! RegisterClassEx(&wc))
    {
        LOG_ERROR("Error registering window class.");
        return NULL;
    }

    window_handle = CreateWindowEx(WINDOW_EXSTYLE, MainWndClass, TEXT(APPNAME),
                                   WINDOW_STYLE,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   100, 300, NULL, NULL, current_instance, NULL);


    if (! window_handle)
    {
        LOG_ERROR("Error creating main window.");
        return NULL;
    }

    return window_handle;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;

    top_window = GetForegroundWindow();


    main_window = createMainWindow(hInstance);
    ShowWindow(main_window, nCmdShow);
    UpdateWindow(main_window);

    //trayicon_init(main_window, APPNAME);

    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

void exitApplication() {
    trayicon_remove();
    PostQuitMessage(0);
}

HWND createMatrixButton(HWND parent, HINSTANCE hInstance, unsigned i, unsigned j) {
    static HWND button;
    button = CreateWindow("button",
                          "",
                          WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_PUSHLIKE|BS_FLAT  ,
                          MARGIN+j*(BUTTON_SIZE+BUTTON_SPACE),MARGIN+i*(BUTTON_SIZE+BUTTON_SPACE),BUTTON_SIZE, BUTTON_SIZE,
                          parent,
                          NULL,
                          hInstance,
                          NULL);//ID_LAST+i*BUTTON_MAX_ROWS+j);
    return button;
}


void initGUI(HWND win, HINSTANCE hInstance) {
    unsigned i,j;

    /*   CreateWindow("button",
                    "Resize!",
                    WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                    0,200,150,50,
                    win,
                    NULL,
                    hInstance,
                    ID_BUTTON_nESIZE);
    */
    for(i=0; i<BUTTON_ROWS; i++)
        for(j=0; j<BUTTON_COLS; j++)
            buttons[i][j] = createMatrixButton(win, hInstance, i, j);

}

bool isResizeAndMoveAble(HWND window) {
    static HWND taskbar, desktop;
    if (!taskbar)
        taskbar  = FindWindow("Shell_TrayWnd", NULL);
    if(!desktop)
        desktop = GetDesktopWindow();
    return GetWindowLongPtr(window, GWL_STYLE) & WS_SIZEBOX && window != taskbar && window != desktop;
}


void resize_and_move_window_to(HWND win, RECT area) {
    RECT workarea, rect ;
    HMONITOR current_monitor;
    MONITORINFO monitor_info;

#if (_WIN32_WINNT >= 0x0500 || _WIN32_WINDOWS >= 0x0410)
    current_monitor = MonitorFromWindow(main_window, MONITOR_DEFAULTTONEAREST );
    monitor_info.cbSize=sizeof(monitor_info);
    GetMonitorInfo(current_monitor, &monitor_info);

    workarea = monitor_info.rcWork;
#else
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);
#endif

    rect.left = workarea.left + (double)(area.left) * (double)(workarea.right-workarea.left)/BUTTON_COLS;
    rect.right = workarea.left + (double)(1+area.right) * (double)(workarea.right-workarea.left)/BUTTON_COLS;
    rect.top = workarea.top + (double)(area.top) * (double)(workarea.bottom -workarea.top)/BUTTON_ROWS;
    rect.bottom = workarea.top + (double)(1+area.bottom) * (double)(workarea.bottom - workarea.top)/BUTTON_ROWS;

    SetWindowPos(win, 0, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 0);
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HINSTANCE hInstance ;
    switch (msg)
    {
    case WM_CREATE: {
        hInstance =((LPCREATESTRUCT) lParam) -> hInstance;
        initGUI(hWnd, hInstance);
        resizeMainWindow(hWnd);
        centerOnWindow(hWnd, top_window);
        return 0;
    }
    /*    case WM_MOUSEMOVE:
        {
            unsigned i,j;

            for(i=0; i<BUTTON_ROWS; i++)
                for(j=0; j<BUTTON_COLS; j++) {
    //          if(lParam == (LPARAM) buttons[i][j]) {
                    SendMessage(buttons[i][j], BM_SETCHECK, BST_CHECKED, 0);
                    SendMessage(buttons[i][j], BM_SETSTYLE, BST_CHECKED, 0);
                    UpdateWindow(buttons[i][j]);
                    return 0;
                    //        }
                }
        }
        break;
    */


    case WM_COMMAND:
    {
        unsigned x,y;

        for(x=0; x<BUTTON_COLS; x++)
            for(y=0; y<BUTTON_ROWS; y++) {
                if(lParam == (LPARAM) buttons[y][x]) {
                    if(is_checking) {
                        RECT area;

                        area.top = min(y, last_click_position.y);
                        area.bottom = max(y, last_click_position.y);

                        area.left = min(x, last_click_position.x);
                        area.right = max(x, last_click_position.x);

                        resize_and_move_window_to(top_window, area);
                    }

                    last_click_position.x = x;
                    last_click_position.y = y;
                }
            }


        is_checking = !is_checking;
        if(!is_checking) {
            // uncheck all buttons
            for(x=0; x<BUTTON_COLS; x++)
                for(y=0; y<BUTTON_ROWS; y++)
                    SendMessage(buttons[y][x], BM_SETCHECK, BST_UNCHECKED, 0);
        }

        return 0;
    }
    break;

    switch (LOWORD(wParam))
    {
    case ID_BUTTON_RESIZE:
        if(HIWORD(wParam) == BN_CLICKED) {
            SetWindowPos(top_window, 0, 0, 0, 100,100,0);
        }
        return 0;

    }
    break;


    case WM_TIMER:
    {
        HWND foreground =  GetForegroundWindow();
        if(foreground != hWnd && isResizeAndMoveAble(foreground)) {
            top_window = foreground;

            centerOnWindow(hWnd, top_window);
            SetFocus(hWnd);
        }
        return 0;
    }

    case WM_KILLFOCUS:
    {
        SetTimer(hWnd, 1, 200, NULL);
        is_checking = false;
        break;
    } 

    case WM_DESTROY:
    {
        exitApplication();
        return 0;
    }

    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}


