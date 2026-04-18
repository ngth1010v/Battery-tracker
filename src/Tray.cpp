#include "Tray.h"
#include "resource.h"

#include <string>
#include <algorithm>

#define ID_POPUP_CHECK_NOTIFICATION 1001
#define ID_POPUP_CHECK_POPUP         1002

static const wchar_t* TRAY_WINDOW_CLASS  = L"BatteryTrayHiddenWindowClass";
static const wchar_t* POPUP_WINDOW_CLASS = L"BatteryTrayTogglePopupClass";

static constexpr int POPUP_WIDTH  = 210;
static constexpr int POPUP_HEIGHT = 84;

static void ClampPopupPosition(int& x, int& y, int width, int height)
{
    POINT pt{ x, y };
    HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);

    if (GetMonitorInfoW(hMon, &mi))
    {
        RECT rc = mi.rcWork;

        if (x + width > rc.right)  x = rc.right - width;
        if (y + height > rc.bottom) y = rc.bottom - height;
        if (x < rc.left) x = rc.left;
        if (y < rc.top) y = rc.top;
    }
}

bool Tray::Init(HINSTANCE hInstance)
{
    s_hInstance = hInstance;

    s_iconOn  = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON_ON));
    s_iconOff = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON_OFF));

    if (!s_iconOn || !s_iconOff)
        return false;

    WNDCLASSEXW wcTray{};
    wcTray.cbSize = sizeof(wcTray);
    wcTray.lpfnWndProc = TrayWndProc;
    wcTray.hInstance = hInstance;
    wcTray.lpszClassName = TRAY_WINDOW_CLASS;

    if (!RegisterClassExW(&wcTray))
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return false;
    }

    WNDCLASSEXW wcPopup{};
    wcPopup.cbSize = sizeof(wcPopup);
    wcPopup.lpfnWndProc = PopupWndProc;
    wcPopup.hInstance = hInstance;
    wcPopup.lpszClassName = POPUP_WINDOW_CLASS;
    wcPopup.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcPopup.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wcPopup))
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return false;
    }

    s_trayWnd = CreateWindowExW(
        0,
        TRAY_WINDOW_CLASS,
        L"BatteryTrayHiddenWindow",
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
    CreatePopupWindow();
    UpdateTrayIcon();

    return true;
}

void Tray::Shutdown()
{
    HidePopupWindow();

    if (s_nid.hWnd)
    {
        Shell_NotifyIconW(NIM_DELETE, &s_nid);
        ZeroMemory(&s_nid, sizeof(s_nid));
    }

    if (s_popupWnd)
    {
        DestroyWindow(s_popupWnd);
        s_popupWnd = nullptr;
    }

    if (s_trayWnd)
    {
        DestroyWindow(s_trayWnd);
        s_trayWnd = nullptr;
    }

    s_btnNotification = nullptr;
    s_btnPopup = nullptr;

    s_iconOn = nullptr;
    s_iconOff = nullptr;
}

bool Tray::IsNotificationActive()
{
    return s_activeNotification;
}

bool Tray::IsPopupActive()
{
    return s_activePopup;
}

bool Tray::IsAnyActive()
{
    return s_activeNotification || s_activePopup;
}

void Tray::SetNotificationActive(bool active)
{
    s_activeNotification = active;
    SyncPopupChecks();
    UpdateTrayIcon();
}

void Tray::SetPopupActive(bool active)
{
    s_activePopup = active;
    SyncPopupChecks();
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

    s_nid.hIcon = IsAnyActive() ? s_iconOn : s_iconOff;
    s_nid.uFlags = NIF_ICON | NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &s_nid);
}

void Tray::CreatePopupWindow()
{
    if (s_popupWnd)
        return;

    s_popupWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        POPUP_WINDOW_CLASS,
        L"",
        WS_POPUP,
        0, 0, POPUP_WIDTH, POPUP_HEIGHT,
        nullptr,
        nullptr,
        s_hInstance,
        nullptr
    );
}

void Tray::SyncPopupChecks()
{
    if (!s_popupWnd)
        return;

    if (s_btnNotification)
        SendMessageW(s_btnNotification, BM_SETCHECK, s_activeNotification ? BST_CHECKED : BST_UNCHECKED, 0);

    if (s_btnPopup)
        SendMessageW(s_btnPopup, BM_SETCHECK, s_activePopup ? BST_CHECKED : BST_UNCHECKED, 0);
}

void Tray::ShowPopupAtCursor()
{
    if (!s_popupWnd)
        return;

    POINT pt{};
    GetCursorPos(&pt);

    int x = pt.x;
    int y = pt.y + 2;
    ClampPopupPosition(x, y, POPUP_WIDTH, POPUP_HEIGHT);

    SetWindowPos(
        s_popupWnd,
        HWND_TOPMOST,
        x, y, POPUP_WIDTH, POPUP_HEIGHT,
        SWP_SHOWWINDOW | SWP_NOACTIVATE
    );

    ShowWindow(s_popupWnd, SW_SHOWNORMAL);
    SetForegroundWindow(s_popupWnd);
    SetFocus(s_popupWnd);
    SyncPopupChecks();
}

void Tray::HidePopupWindow()
{
    if (s_popupWnd && IsWindowVisible(s_popupWnd))
        ShowWindow(s_popupWnd, SW_HIDE);
}

void Tray::UpdateToggleStateFromCheckbox(int controlId)
{
    if (controlId == ID_POPUP_CHECK_NOTIFICATION && s_btnNotification)
    {
        BOOL checked = (SendMessageW(s_btnNotification, BM_GETCHECK, 0, 0) == BST_CHECKED);
        SetNotificationActive(checked);
    }
    else if (controlId == ID_POPUP_CHECK_POPUP && s_btnPopup)
    {
        BOOL checked = (SendMessageW(s_btnPopup, BM_GETCHECK, 0, 0) == BST_CHECKED);
        SetPopupActive(checked);
    }
}

LRESULT CALLBACK Tray::PopupWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        s_btnNotification = CreateWindowExW(
            0,
            L"BUTTON",
            L"Notification",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
            12, 10, 170, 22,
            hwnd,
            (HMENU)(INT_PTR)ID_POPUP_CHECK_NOTIFICATION,
            s_hInstance,
            nullptr
        );

        s_btnPopup = CreateWindowExW(
            0,
            L"BUTTON",
            L"Popup",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
            12, 38, 170, 22,
            hwnd,
            (HMENU)(INT_PTR)ID_POPUP_CHECK_POPUP,
            s_hInstance,
            nullptr
        );

        SyncPopupChecks();
        return 0;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        if (code == BN_CLICKED)
        {
            UpdateToggleStateFromCheckbox(id);
            return 0;
        }
        break;
    }

    case WM_KILLFOCUS:
        HidePopupWindow();
        return 0;

    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
            HidePopupWindow();
        return 0;

    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc{};
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CLOSE:
        HidePopupWindow();
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK Tray::TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_TRAYICON_MSG) {
        if (lParam == WM_LBUTTONUP)
        {
            // toggle state
            SetNotificationActive(!IsNotificationActive());
            SetPopupActive(!IsPopupActive());

            UpdateTrayIcon();

            return 0;
        }

        if (lParam == WM_RBUTTONUP)
        {
            ShowPopupAtCursor();
            return 0;
        }
    }

    if (msg == WM_DESTROY){
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}