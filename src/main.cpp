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

// ======================================================================================================================
// CONFIG
// ======================================================================================================================
const int LOW_THRESHOLD                     = 30;
const int WARNING_LOW_THRESHOLD             = 40;
const int WARNING_HIGH_THRESHOLD            = 80;
const int HIGH_THRESHOLD                    = 90;

const int INTERVAL_CHECK                    = 5000;
const int INTERVAL_CHECK_WHEN_WARNING       = 1000;

const bool ACTIVE_NOFICATION                = true;
const int REPEAT_NOFICATION_AFTER_PERCENT   = 5;

const bool ACTIVE_POPUP                     = true;
const float POPUP_SCALE                     = 1;

const bool DEBUGING                         = false;

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
std::string checkWarning(int percent, bool charging){
    if (!charging && percent <= LOW_THRESHOLD)           return "L";
    if (!charging && percent <= WARNING_LOW_THRESHOLD)   return "WL";
    if (charging  && percent >= HIGH_THRESHOLD)          return "H";
    if (charging  && percent >= WARNING_HIGH_THRESHOLD)  return "WH";
    return "";
}

std::string getTitle(std::string warningState){
    if (warningState == "L")  return "Battery too low";
    if (warningState == "WL") return "Low battery";
    if (warningState == "WH") return "High battery";
    if (warningState == "H")  return "Battery too high";
    return "";
}

std::string getDesc(std::string warningState){
    if (warningState == "L" || warningState == "WL") return "Plug in now!";
    if (warningState == "H" || warningState == "WH") return "Unplug now!";
    return "";
}

// Delay KHÔNG BLOCK hoàn toàn
void delaySmart(const std::string& warningState){
    int total = (warningState != "") ? INTERVAL_CHECK_WHEN_WARNING : INTERVAL_CHECK;

    int step = 100; // check mỗi 100ms
    int elapsed = 0;

    while (elapsed < total && g_running) {
        // check event từ ngoài
        if (g_exitEvent && WaitForSingleObject(g_exitEvent, 0) == WAIT_OBJECT_0) {
            g_running = false;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(step));
        elapsed += step;
    }
}

// ======================================================================================================================
// MAIN
// ======================================================================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    // Setup console handler
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    // External exit event (optional)
    g_exitEvent = CreateEvent(NULL, TRUE, FALSE, L"BatteryTrackerExitEvent");

    // General
    int percent                     = 0;
    int prevPercent                 = 0;
    bool charging                   = false;
    std::string warningState        = "";
    std::string prevWarningState    = "";

    // Notification
    int prevNotificationPercent     = 0;

    // Popup
    bool popupOpening               = false;

    if (Battery::HasBattery() || DEBUGING){

        // Init
        WarningIcon::Init();
        Notification::Init();
        Popup::Init(hInstance, POPUP_SCALE);

        // MAIN LOOP
        while (g_running) {

            // ---------------------------------------------------------------------------------------------------------
            // MESSAGE LOOP
            // ---------------------------------------------------------------------------------------------------------
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

                if (msg.message == WM_QUIT) {
                    g_running = false;
                    break;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (!g_running) break;

            std::cout << "Check...\n";

            // ---------------------------------------------------------------------------------------------------------
            // GET DATA
            // ---------------------------------------------------------------------------------------------------------
            prevPercent = percent;
            percent = Battery::GetBatteryPercent();
            charging = Battery::IsCharging();

            if (DEBUGING) {
                std::cout << "[DEBUG] Input percent: ";
                std::cin >> percent;
                std::cout << "[DEBUG] Input is charging (y/n): ";
                std::string inp;
                std::cin >> inp;
                charging = (inp == "y");
            }

            prevWarningState = warningState;
            warningState = checkWarning(percent, charging);

            // ---------------------------------------------------------------------------------------------------------
            // NOTIFICATION
            // ---------------------------------------------------------------------------------------------------------
            if (ACTIVE_NOFICATION){
                if (!warningState.empty()){
                    if (warningState != prevWarningState ||
                        abs(percent - prevNotificationPercent) >= REPEAT_NOFICATION_AFTER_PERCENT) 
                    {
                        std::string title = getTitle(warningState) + " (" + std::to_string(percent) + "%)";
                        Notification::Show(title, getDesc(warningState));
                        prevNotificationPercent = percent;
                    }
                }
            }

            // ---------------------------------------------------------------------------------------------------------
            // POPUP
            // ---------------------------------------------------------------------------------------------------------
            if (ACTIVE_POPUP){
                if (!warningState.empty()){
                    popupOpening = true;

                    if (warningState != prevWarningState || percent != prevPercent){
                        Popup::Show(
                            percent,
                            getTitle(warningState),
                            getDesc(warningState),
                            LOW_THRESHOLD,
                            WARNING_LOW_THRESHOLD,
                            WARNING_HIGH_THRESHOLD,
                            HIGH_THRESHOLD
                        );
                    }
                }
                else {
                    if (popupOpening){
                        Popup::Hide();
                        popupOpening = false;
                    }
                }
            }

            std::cout << "Result: " << warningState << "\n\n";

            // ---------------------------------------------------------------------------------------------------------
            // DELAY (non-blocking)
            // ---------------------------------------------------------------------------------------------------------
            delaySmart(warningState);
        }

        // =============================================================================================================
        // CLEANUP
        // =============================================================================================================
        std::cout << "Shutting down...\n";

        Popup::Hide();
        Popup::Shutdown();
        WarningIcon::Shutdown();
    }
    else {
        std::cout << "No battery found!";
    }

    if (g_exitEvent) {
        CloseHandle(g_exitEvent);
        g_exitEvent = NULL;
    }

    return 0;
}