#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <windows.h>

#include "batteryApi.h"
#include "WarningIcon.h"
#include "Notification.h"
#include "Popup.h"
#include "ConfigManager.h"
#include "Tray.h"



// ======================================================================================================================
// GLOBAL CONTROL
// ======================================================================================================================
bool g_running = true;
HANDLE g_exitEvent = NULL;



// ======================================================================================================================
// CONSOLE HANDLER
// ======================================================================================================================
BOOL WINAPI ConsoleHandler(DWORD signal) {
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
}



// ======================================================================================================================
// UTILS
// ======================================================================================================================
std::string checkWarning(int percent, bool charging, const ConfigManager::DataPack& cfg) {
    if (!charging && percent <= cfg.LOW_THRESHOLD) return "L";
    if (!charging && percent <= cfg.WARNING_LOW_THRESHOLD) return "WL";
    if (charging && percent >= cfg.HIGH_THRESHOLD) return "H";
    if (charging && percent >= cfg.WARNING_HIGH_THRESHOLD) return "WH";
    return "";
}

std::string getTitle(const std::string& warningState) {
    if (warningState == "L")  return "Battery too low";
    if (warningState == "WL") return "Low battery";
    if (warningState == "WH") return "High battery";
    if (warningState == "H")  return "Battery too high";
    return "";
}

std::string getDesc(const std::string& warningState) {
    if (warningState == "L" || warningState == "WL") return "Plug in now!";
    if (warningState == "H" || warningState == "WH") return "Unplug now!";
    return "";
}

// Delay KHÔNG BLOCK hoàn toàn
void delaySmart(const std::string& warningState, const ConfigManager::DataPack& cfg)
{
    int total = (warningState != "") ?
        cfg.INTERVAL_CHECK_WHEN_WARNING :
        cfg.INTERVAL_CHECK;

    DWORD start = GetTickCount();

    while (g_running)
    {
        // 1. EXIT EVENT
        if (g_exitEvent &&
            WaitForSingleObject(g_exitEvent, 0) == WAIT_OBJECT_0)
        {
            g_running = false;
            break;
        }

        // 2. CONFIG CHANGE → WAKE IMMEDIATELY
        if (g_configEvent &&
            WaitForSingleObject(g_configEvent, 0) == WAIT_OBJECT_0)
        {
            ResetEvent(g_configEvent);
            break; // 👉 WAKE MAIN LOOP NGAY
        }

        // 3. ENOUGH DELAY TIME → EXIT
        if ((int)(GetTickCount() - start) >= total)
            break;

        Sleep(10); // nhẹ CPU, nhưng vẫn responsive
    }
}



// ======================================================================================================================
// MAIN
// ======================================================================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    g_exitEvent = CreateEvent(NULL, TRUE, FALSE, L"BatteryTrackerExitEvent");
    g_configEvent = CreateEvent(NULL, TRUE, FALSE, L"BatteryTrackerConfigEvent");

    ConfigManager& configManager = ConfigManager::Instance();
    const auto& cfg = configManager.get();

    int percent = 0;
    int prevPercent = 0;
    bool charging = false;

    std::string warningState = "";
    std::string prevWarningState = "";

    int prevNotificationPercent = 0;
    bool popupOpening = false;



    if (Battery::HasBattery() || cfg.DEBUGGING)
    {
        WarningIcon::Init();
        Notification::Init();
        Popup::Init(hInstance, cfg.POPUP_SCALE);
        Tray::Init(hInstance);

        while (g_running)
        {
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    g_running = false;
                    break;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (!g_running) break;

            std::cout << "Check...\n";

            prevPercent = percent;
            percent = Battery::GetBatteryPercent();
            charging = Battery::IsCharging();

            if (cfg.DEBUGGING)
            {
                std::cout << "[DEBUG] Input percent: ";
                std::cin >> percent;

                std::cout << "[DEBUG] Input is charging (y/n): ";
                std::string inp;
                std::cin >> inp;
                charging = (inp == "y");
            }

            prevWarningState = warningState;
            warningState = checkWarning(percent, charging, cfg);

            // =========================
            // NOTIFICATION
            // =========================
            if (cfg.ACTIVE_NOTIFICATION && !warningState.empty())
            {
                if (warningState != prevWarningState ||
                    abs(percent - prevNotificationPercent) >= cfg.REPEAT_NOTIFICATION_AFTER_PERCENT)
                {
                    Notification::Show(
                        getTitle(warningState) + " (" + std::to_string(percent) + "%)",
                        getDesc(warningState)
                    );

                    prevNotificationPercent = percent;
                }
            }

            // =========================
            // POPUP
            // =========================
            if (cfg.ACTIVE_POPUP)
            {
                if (!warningState.empty())
                {
                    if (warningState != prevWarningState || percent != prevPercent || !popupOpening)
                    {
                        Popup::Show(
                            percent,
                            getTitle(warningState),
                            getDesc(warningState),
                            cfg.LOW_THRESHOLD,
                            cfg.WARNING_LOW_THRESHOLD,
                            cfg.WARNING_HIGH_THRESHOLD,
                            cfg.HIGH_THRESHOLD
                        );
                    }
                    popupOpening = true;
                }
                else
                {
                    if (popupOpening)
                    {
                        Popup::Hide();
                        popupOpening = false;
                    }
                }
            }
            else {
                std::cout << "LOGGGG\n";
                if (popupOpening)
                {
                    Popup::Hide();
                    popupOpening = false;
                }
            }

            std::cout << "Result: " << warningState << "\n\n";

            // =========================
            // INTERRUPTIBLE DELAY
            // =========================
            delaySmart(warningState, cfg);
        }

        std::cout << "Shutting down...\n";

        Popup::Hide();
        Popup::Shutdown();
        WarningIcon::Shutdown();
    }
    else
    {
        std::cout << "No battery found!";
    }

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

    return 0;
}



