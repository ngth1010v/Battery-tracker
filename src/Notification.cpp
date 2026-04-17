#include "Notification.h"
#include <windows.h>
#include <shellapi.h>
#include <string>

#pragma comment(lib, "shell32.lib")

static NOTIFYICONDATA nid = {};
static bool initialized = false;

static std::wstring ToW(const std::string& s)
{
    return std::wstring(s.begin(), s.end());
}

void Notification::Init()
{
    if (initialized) return;

    ZeroMemory(&nid, sizeof(nid));

    nid.cbSize = sizeof(nid);
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uID = 1;

    nid.hIcon = LoadIcon(NULL, IDI_WARNING); // system warning icon
    nid.hWnd = GetConsoleWindow(); // hoặc HWND app của bạn

    wcscpy(nid.szTip, L"BatteryTracker");

    Shell_NotifyIcon(NIM_ADD, &nid);

    initialized = true;
}

void Notification::Show(const std::string& title,
                        const std::string& message)
{
    if (!initialized) Init();

    NOTIFYICONDATA notify = {};
    notify.cbSize = sizeof(notify);
    notify.hWnd = nid.hWnd;
    notify.uID = nid.uID;
    notify.uFlags = NIF_INFO;

    std::wstring wTitle = ToW(title);
    std::wstring wMsg   = ToW(message);

    wcscpy(notify.szInfoTitle, wTitle.c_str());
    wcscpy(notify.szInfo, wMsg.c_str());

    notify.dwInfoFlags = NIIF_WARNING; // ⚠ system warning icon

    Shell_NotifyIcon(NIM_MODIFY, &notify);
}