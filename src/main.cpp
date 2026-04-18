#include <iostream>
#include <string>
#include <windows.h>

#include "batteryApi.h"
#include "WarningIcon.h"
#include "Notification.h"
#include "Popup.h"
#include "ConfigManager.h"
#include "Tray.h"

// ======================================================
// GLOBAL STATE (shared between functions)
// ======================================================
bool g_running = true;

HANDLE g_exitEvent = NULL;
HANDLE g_configEvent = NULL;

ConfigManager* g_cfg = nullptr;

int g_percent = 0;
int g_prevPercent = 0;
bool g_charging = false;

std::string g_warningState = "";
std::string g_prevWarningState = "";

int g_prevNotificationPercent = 0;

bool g_popupOpening = false;

ULONGLONG g_lastCheckTimestamp = 0;




// ======================================================
// UTILS
// ======================================================
std::string checkWarning(int percent, bool charging, const ConfigManager::DataPack& cfg)
{
    if (!charging && percent <= cfg.LOW_THRESHOLD) return "L";
    if (!charging && percent <= cfg.WARNING_LOW_THRESHOLD) return "WL";
    if (charging && percent >= cfg.HIGH_THRESHOLD) return "H";
    if (charging && percent >= cfg.WARNING_HIGH_THRESHOLD) return "WH";
    return "";
}

std::string getTitle(const std::string& s)
{
    if (s == "L") return "Battery too low";
    if (s == "WL") return "Low battery";
    if (s == "H") return "Battery too high";
    if (s == "WH") return "High battery";
    return "";
}

std::string getDesc(const std::string& s)
{
    if (s == "L" || s == "WL") return "Plug in now!";
    if (s == "H" || s == "WH") return "Unplug now!";
    return "";
}




// ======================================================
// INIT
// ======================================================
bool init(HINSTANCE hInstance)
{
    SetConsoleCtrlHandler([](DWORD signal)->BOOL {
        if (signal == CTRL_C_EVENT ||
            signal == CTRL_CLOSE_EVENT ||
            signal == CTRL_SHUTDOWN_EVENT ||
            signal == CTRL_LOGOFF_EVENT)
        {
            g_running = false;
            if (g_exitEvent) SetEvent(g_exitEvent);
            return TRUE;
        }
        return FALSE;
    }, TRUE);

    g_exitEvent = CreateEvent(NULL, TRUE, FALSE, L"BatteryTrackerExitEvent");
    g_configEvent = CreateEvent(NULL, TRUE, FALSE, L"BatteryTrackerConfigEvent");

    g_cfg = &ConfigManager::Instance();

    const auto& cfg = g_cfg->get();

    WarningIcon::Init();
    Notification::Init();
    Popup::Init(hInstance, cfg.POPUP_SCALE);
    Tray::Init(hInstance);

    return true;
}


// ======================================================
// EVENT HANDLE (logic + timing)
// ======================================================
void eventHandle()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            g_running = false;
            return;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_exitEvent && WaitForSingleObject(g_exitEvent, 0) == WAIT_OBJECT_0)
    {
        g_running = false;
        return;
    }

    if (g_configEvent && WaitForSingleObject(g_configEvent, 0) == WAIT_OBJECT_0)
    {
        ResetEvent(g_configEvent);
        g_lastCheckTimestamp = 0; // force recheck
    }
}



// ======================================================
// RENDER / LOGIC UPDATE
// ======================================================
void render()
{
    const auto& cfg = g_cfg->get();

    ULONGLONG now = GetTickCount64();

    int interval = cfg.INTERVAL_CHECK;


    if (g_lastCheckTimestamp != 0 &&
        (now - g_lastCheckTimestamp) < (ULONGLONG)interval)
    {
        return;
    }

    // ======================
    // READ BATTERY
    // ======================
    g_prevPercent = g_percent;
    g_percent = Battery::GetBatteryPercent();
    g_charging = Battery::IsCharging();

    // ======================
    // STATE UPDATE
    // ======================
    g_prevWarningState = g_warningState;
    g_warningState = checkWarning(g_percent, g_charging, cfg);

    // ======================
    // NOTIFICATION
    // ======================
    if (Tray::IsActive() && !g_warningState.empty() && cfg.ACTIVE_NOTIFICATION)
    {
        if (g_warningState != g_prevWarningState ||
            abs(g_percent - g_prevNotificationPercent) >= cfg.REPEAT_NOTIFICATION_AFTER_PERCENT)
        {
            Notification::Show(
                getTitle(g_warningState) + " (" + std::to_string(g_percent) + "%)",
                getDesc(g_warningState)
            );

            g_prevNotificationPercent = g_percent;
        }
    }

    // ======================
    // POPUP
    // ======================
    if (Tray::IsActive() && cfg.ACTIVE_POPUP)
    {
        if (!g_warningState.empty())
        {
            if (g_warningState != g_prevWarningState ||
                g_percent != g_prevPercent ||
                !g_popupOpening)
            {
                Popup::Show(
                    g_percent,
                    getTitle(g_warningState),
                    getDesc(g_warningState),
                    cfg.LOW_THRESHOLD,
                    cfg.WARNING_LOW_THRESHOLD,
                    cfg.WARNING_HIGH_THRESHOLD,
                    cfg.HIGH_THRESHOLD
                );
            }

            g_popupOpening = true;
        }
        else
        {
            if (g_popupOpening)
            {
                Popup::Hide();
                g_popupOpening = false;
            }
        }
    }
    else
    {
        if (g_popupOpening)
        {
            Popup::Hide();
            g_popupOpening = false;
        }
    }

    g_lastCheckTimestamp = GetTickCount64();
}



// ======================================================
// DESTROY
// ======================================================
void destroy()
{
    std::cout << "Shutting down...\n";

    Popup::Hide();
    Popup::Shutdown();
    WarningIcon::Shutdown();
    Tray::Shutdown();

    if (g_exitEvent)
    {
        CloseHandle(g_exitEvent);
        g_exitEvent = NULL;
    }

    if (g_configEvent)
    {
        CloseHandle(g_configEvent);
        g_configEvent = NULL;
    }
}



// ======================================================
// MAIN LOOP
// ======================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{

    if (!Battery::HasBattery())
    {
        std::cout << "No battery found!";
        return 0;
    }

    init(hInstance);

    while (g_running)
    {
        eventHandle();
        if (!g_running) break;

        render();
    }

    destroy();
    return 0;
}