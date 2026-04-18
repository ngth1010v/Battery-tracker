#include "Tray.h"
#include "resource.h"

static const wchar_t* TRAY_WINDOW_CLASS = L"BatteryTrayHiddenWindowClass";

bool Tray::Init(HINSTANCE hInstance)
{
    s_hInstance = hInstance;

    s_iconOn  = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON_ON));
    s_iconOff = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON_OFF));

    if (!s_iconOn || !s_iconOff)
        return false;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TRAY_WINDOW_CLASS;

    if (!RegisterClassExW(&wc))
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return false;
    }

    s_trayWnd = CreateWindowExW(
        0,
        TRAY_WINDOW_CLASS,
        L"TrayHiddenWindow",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        hInstance,
        nullptr
    );

    if (!s_trayWnd)
        return false;

    CreateTrayIcon();
    UpdateTrayIcon();

    return true;
}

bool Tray::IsActive()
{
    return s_active;
}

void Tray::SetActive(bool active)
{
    s_active = active;
    UpdateTrayIcon();
}

void Tray::CreateTrayIcon()
{
    ZeroMemory(&s_nid, sizeof(s_nid));

    s_nid.cbSize = sizeof(NOTIFYICONDATAW);
    s_nid.hWnd = s_trayWnd;
    s_nid.uID = 1;
    s_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    s_nid.uCallbackMessage = WM_TRAYICON_MSG;
    s_nid.hIcon = s_iconOff;

    wcscpy_s(s_nid.szTip, L"Battery Tracker");

    Shell_NotifyIconW(NIM_ADD, &s_nid);
}

void Tray::UpdateTrayIcon()
{
    if (!s_nid.hWnd)
        return;

    s_nid.hIcon = s_active ? s_iconOn : s_iconOff;
    s_nid.uFlags = NIF_ICON | NIF_TIP;

    Shell_NotifyIconW(NIM_MODIFY, &s_nid);
}

LRESULT CALLBACK Tray::TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_TRAYICON_MSG)
    {
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
        {
            SetActive(!IsActive());
            return 0;
        }
    }

    if (msg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void Tray::Shutdown()
{
    if (s_nid.hWnd)
    {
        Shell_NotifyIconW(NIM_DELETE, &s_nid);
        ZeroMemory(&s_nid, sizeof(s_nid));
    }

    if (s_trayWnd)
    {
        DestroyWindow(s_trayWnd);
        s_trayWnd = nullptr;
    }

    s_iconOn = nullptr;
    s_iconOff = nullptr;
}