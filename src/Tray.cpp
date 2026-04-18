#include "Tray.h"
#include "ConfigManager.h"
#include "resource.h"

#define ID_MENU_SETTINGS 1
#define ID_MENU_EXIT     2

// ======================================================
// INIT
// ======================================================
bool Tray::Init(HINSTANCE hInstance)
{
    s_hInstance = hInstance;

    // Load icons from .rc
    s_iconOn  = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_ON));
    s_iconOff = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_OFF));

    if (!s_iconOn || !s_iconOff)
        return false;

    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"BatteryTrayWindow";

    RegisterClass(&wc);

    // Hidden message-only window
    s_hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        L"TrayHiddenWindow",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        hInstance,
        nullptr
    );

    if (!s_hwnd)
        return false;

    CreateTrayIcon();
    UpdateTrayIcon();

    return true;
}

// ======================================================
// SHUTDOWN
// ======================================================
void Tray::Shutdown()
{
    Shell_NotifyIcon(NIM_DELETE, &s_nid);

    if (s_iconOn) DestroyIcon(s_iconOn);
    if (s_iconOff) DestroyIcon(s_iconOff);
}

// ======================================================
// CREATE TRAY ICON
// ======================================================
void Tray::CreateTrayIcon()
{
    ZeroMemory(&s_nid, sizeof(s_nid));

    s_nid.cbSize = sizeof(NOTIFYICONDATA);
    s_nid.hWnd = s_hwnd;
    s_nid.uID = 1;
    s_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    s_nid.uCallbackMessage = WM_TRAYICON_MSG;

    wcscpy_s(s_nid.szTip, L"Battery Tracker");

    s_nid.hIcon = s_iconOff;

    Shell_NotifyIcon(NIM_ADD, &s_nid);
}

// ======================================================
// STATE CHECK
// ======================================================
bool Tray::IsActive()
{
    const auto& cfg = ConfigManager::Instance().get();
    return cfg.ACTIVE_NOTIFICATION || cfg.ACTIVE_POPUP;
}

// ======================================================
// UPDATE ICON
// ======================================================
void Tray::UpdateTrayIcon()
{
    s_nid.hIcon = IsActive() ? s_iconOn : s_iconOff;
    Shell_NotifyIcon(NIM_MODIFY, &s_nid);
}

// ======================================================
// APPLY TOGGLE LOGIC
// ======================================================
void Tray::ApplyToggle()
{
    auto& cfg = const_cast<ConfigManager::DataPack&>(
        ConfigManager::Instance().get()
    );

    bool active = cfg.ACTIVE_NOTIFICATION || cfg.ACTIVE_POPUP;

    if (active)
    {
        cfg.ACTIVE_NOTIFICATION = false;
        cfg.ACTIVE_POPUP = false;
    }
    else
    {
        cfg.ACTIVE_NOTIFICATION = true;
        cfg.ACTIVE_POPUP = true;
    }

    ConfigManager::Instance().set(cfg);

    UpdateTrayIcon();
}

// ======================================================
// CONTEXT MENU
// ======================================================
void Tray::ShowContextMenu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU menu = CreatePopupMenu();

    AppendMenu(menu, MF_STRING, ID_MENU_SETTINGS, L"Settings");
    AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(menu, MF_STRING, ID_MENU_EXIT, L"Exit");

    SetForegroundWindow(hwnd);

    int cmd = TrackPopupMenu(
        menu,
        TPM_RETURNCMD | TPM_RIGHTBUTTON,
        pt.x,
        pt.y,
        0,
        hwnd,
        NULL
    );

    DestroyMenu(menu);

    switch (cmd)
    {
    case ID_MENU_SETTINGS:
        // placeholder hook
        MessageBox(hwnd, L"Open Settings (TODO)", L"Tray", MB_OK);
        break;

    case ID_MENU_EXIT:
        PostQuitMessage(0);
        break;
    }
}

// ======================================================
// WND PROC
// ======================================================
LRESULT CALLBACK Tray::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TRAYICON_MSG:
    {
        if (lParam == WM_LBUTTONDOWN)
        {
            ApplyToggle();
        }
        else if (lParam == WM_RBUTTONDOWN)
        {
            ShowContextMenu(hwnd);
        }
    }
    break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}