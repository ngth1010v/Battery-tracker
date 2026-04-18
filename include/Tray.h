#pragma once
#include <windows.h>
#include <shellapi.h>

#define WM_TRAYICON_MSG (WM_USER + 1)

class Tray {
public:
    static bool Init(HINSTANCE hInstance);
    static void Shutdown();

    static bool IsNotificationActive();
    static bool IsPopupActive();
    static bool IsAnyActive();

    static void SetNotificationActive(bool active);
    static void SetPopupActive(bool active);

private:
    static LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK PopupWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static void CreateTrayIcon();
    static void UpdateTrayIcon();

    static void CreatePopupWindow();
    static void ShowPopupAtCursor();
    static void HidePopupWindow();
    static void SyncPopupChecks();

    static void UpdateToggleStateFromCheckbox(int controlId);

private:
    static inline HWND s_trayWnd = nullptr;
    static inline HWND s_popupWnd = nullptr;
    static inline HWND s_btnNotification = nullptr;
    static inline HWND s_btnPopup = nullptr;

    static inline HINSTANCE s_hInstance = nullptr;
    static inline NOTIFYICONDATAW s_nid{};

    static inline HICON s_iconOn = nullptr;
    static inline HICON s_iconOff = nullptr;

    static inline bool s_activeNotification = true;
    static inline bool s_activePopup = true;
};