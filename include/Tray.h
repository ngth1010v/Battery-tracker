#pragma once
#include <windows.h>
#include <shellapi.h>

#define WM_TRAYICON_MSG (WM_USER + 1)

class Tray {
public:
    static bool Init(HINSTANCE hInstance);
    static void Shutdown();

    static bool IsActive();
    static void SetActive(bool active);

private:
    static LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static void CreateTrayIcon();
    static void UpdateTrayIcon();

private:
    static inline HWND s_trayWnd = nullptr;

    static inline HINSTANCE s_hInstance = nullptr;
    static inline NOTIFYICONDATAW s_nid{};

    static inline HICON s_iconOn = nullptr;
    static inline HICON s_iconOff = nullptr;

    static inline bool s_active = true;
};