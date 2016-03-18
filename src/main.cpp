
#include "resource.h"

#include <Windows.h>
#include <time.h>
//#include <Commctrl.h>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")

#include "toCommon.h"

//typedef HRESULT (*lpfnLoadIconMetric)(HINSTANCE hinst, PCWSTR pszName, int lims, HICON *phico);
//lpfnLoadIconMetric loadIconMetric;

#define WNDCLASSNAME L"Sowicm's Tray Windows"

#define WM_NOTIFYICON WM_APP

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK InformDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HINSTANCE g_hInst;
NOTIFYICONDATA nid;
HANDLE hStdInR, hStdInW, hStdOutR, hStdOutW, hStdErrR, hStdErrW;

#define IDT_StartCheck 1
#define IDT_Check      2
#define IDT_DestroyDlgInform 3

#define Check_Interval 500

bool bInformed = false;
HWND g_hDlgInform;

void CALLBACK TimerProc(HWND hWnd, UINT message, UINT uID, DWORD dwTime)
{
    if (uID == IDT_StartCheck)
    {
        KillTimer(hWnd, IDT_StartCheck);
        SetTimer(hWnd, IDT_Check, Check_Interval, TimerProc);
    }
    else if (uID == IDT_Check)
    {
        time_t t;
        time(&t);
        tm *ptm = localtime(&t);

        if (ptm->tm_hour == 23 && ptm->tm_min >= 30)
        {
            if (!bInformed)
            {
                bInformed = true;
                g_hDlgInform = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, InformDlgProc);
                DWORD Err = GetLastError();
                ShowWindow(g_hDlgInform, SW_NORMAL);
                //SetTimer(hWnd, IDT_DestroyDlgInform, 3 * 60 * 1000, TimerProc);
            }
        }
        #if 0
        else if (ptm->tm_hour >= 0 && ptm->tm_hour < 8)
        {
            KillTimer(hWnd, IDT_Check);
            SetTimer(hWnd, IDT_StartCheck, 10 * 60 * 1000, TimerProc);
            bInformed = false;

            STARTUPINFOA si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.hStdError = hStdErrW;
            si.hStdOutput = hStdOutW;
            si.hStdInput = hStdOutR;
            si.dwFlags = STARTF_USESTDHANDLES;
            ZeroMemory(&pi, sizeof(pi));

            if (CreateProcessA(NULL, "shutdown -h -f", NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
            {
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
            else
            {
                ShellExecute(NULL, L"open", L"shutdown", L"-h -f", NULL, SW_NORMAL);
            }
        }
        #endif
    }
    /*
    else if (uID == IDT_DestroyDlgInform)
    {
        KillTimer(hWnd, IDT_DestroyDlgInform);
        DestroyWindow(g_hDlgInform);
    }
    */
}

void initData()
{
    //wchar_t path[MAX_PATH + 1];

    //SHGetSpecialFolderLocation(NULL, CSIDL_STARTUP, &pidl);

    //SHGetPathFromIDList(pidl, path);
}

int APIENTRY wWinMain(HINSTANCE hInst,
                      HINSTANCE hPrevInst,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    setAutoRun();

    //{ Mutex
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"AoYeKeXingA");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(hMutex);
        MessageBoxW(NULL, L"干嘛同时运行2个呢？", L"Ops!", MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    //} Mutex

    g_hInst = hInst;

    //{ BEGIN PIPES

    SECURITY_ATTRIBUTES sa;

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = NULL;


    CreatePipe(&hStdInR, &hStdInW, &sa, 0);
    CreatePipe(&hStdOutR, &hStdOutW, &sa, 0);
    CreatePipe(&hStdErrR, &hStdErrW, &sa, 0);

    SetHandleInformation(hStdOutR, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hStdInW, HANDLE_FLAG_INHERIT, 0);

    // } PIPES END

    /*
    HMODULE hModule = GetModuleHandleW(L"Comctl32.dll");
    if (hModule == NULL)
        MessageBox(NULL, L"Fuck Module", L"Fuck", MB_OK);
    loadIconMetric = (lpfnLoadIconMetric)GetProcAddress(hModule, "LoadIconMetric");
    if (loadIconMetric == NULL)
        MessageBox(NULL, L"Fuck", L"Fuck", MB_OK);
    */

    WNDCLASSEXW wndCls;
    wndCls.cbSize        = sizeof(wndCls);
    wndCls.hInstance     = hInst;
    wndCls.lpszClassName = WNDCLASSNAME;
    wndCls.style         = 0;//CS_HREDRAW | CS_VREDRAW;
    wndCls.lpfnWndProc   = WndProc;
    wndCls.hIcon         = NULL;
    wndCls.hIconSm       = NULL;
    wndCls.hCursor       = NULL;
    wndCls.cbClsExtra    = 0;
    wndCls.cbWndExtra    = 0;
    wndCls.hbrBackground = NULL;
    wndCls.lpszMenuName  = NULL;

    RegisterClassExW(&wndCls);

    HWND hWnd = CreateWindowExW(0,
                                WNDCLASSNAME,
                                NULL,
                                0,
                                0,
                                0,
                                0,
                                0,
                                HWND_MESSAGE,
                                NULL,
                                hInst,
                                NULL);

    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.hIcon = (HICON)LoadImageW(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR);
        //LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    //loadIconMetric(hInst, MAKEINTRESOURCE(IDI_ICON1), LIM_SMALL, &(nid.hIcon));
    nid.uCallbackMessage = WM_NOTIFYICON;
    wcscpy(nid.szTip, L"熬夜克星A\r\nWelcome to sowicm.com");
    Shell_NotifyIcon(NIM_ADD, &nid);

    //nid.uVersion = NOTIFYICON_VERSION;

    initData();

    SetTimer(hWnd, IDT_Check, Check_Interval, TimerProc);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        //if (!IsWindow(g_hDlgInform) || !IsDialogMessage(g_hDlgInform, &msg))
        //{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        //}
    }

    CloseHandle(hMutex);

    return msg.wParam;
}

void showContextMenu(HWND hWnd)
{
    POINT pt;
	GetCursorPos(&pt);

    HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU1));

    //SetForegroundWindow(hWnd);
    TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_NOTIFYICON:
        switch (lParam)
        {
        case WM_LBUTTONDBLCLK:
            break;
        case WM_RBUTTONDOWN:
        case WM_CONTEXTMENU:
            showContextMenu(hWnd);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_Page:
            ShellExecute(NULL, L"open", L"http://sowicm.com", NULL, NULL, SW_NORMAL);
            break;

        case ID_Weibo:
            ShellExecute(NULL, L"open", L"http://weibo.com/zhizunmingshuai", NULL, NULL, SW_NORMAL);
            break;
        }
        break;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        break;
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

INT_PTR CALLBACK InformDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            HWND hDesktop = GetDesktopWindow();
            RECT rcDesktop, rcDlg;
            GetWindowRect(hDesktop, &rcDesktop);
            GetWindowRect(hWndDlg, &rcDlg);

            SetWindowPos(hWndDlg,
                HWND_TOP,
                rcDesktop.left + (RectWidth(rcDesktop) - RectWidth(rcDlg)) / 2,
                rcDesktop.top  + (RectHeight(rcDesktop) - RectHeight(rcDlg)) / 2,
                0 ,0,
                SWP_NOSIZE);

            return TRUE;
        }

    case WM_CLOSE:
        DestroyWindow(hWndDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            DestroyWindow(hWndDlg);
            return TRUE;
        }
    }
    return FALSE;
}
