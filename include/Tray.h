#pragma once
#include <windows.h>
#include <shellapi.h>
#include <functional>

#define WM_TRAYICON_MSG (WM_USER + 1)

class Tray {
public:
    static bool Init(HINSTANCE hInstance);
    static void Shutdown();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static void CreateTrayIcon();
    static void UpdateTrayIcon();
    static void ShowContextMenu(HWND hwnd);

    static void ApplyToggle();
    static bool IsActive();

private:
    static inline HWND s_hwnd = nullptr;
    static inline HINSTANCE s_hInstance = nullptr;

    static inline NOTIFYICONDATA s_nid;

    static inline HICON s_iconOn = nullptr;
    static inline HICON s_iconOff = nullptr;
};