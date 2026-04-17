#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib> 
#include "batteryApi.h"
#include "WarningIcon.h"
#include "Notification.h"
#include "Popup.h"

// ======================================================================================================================
// CONFIG
// ======================================================================================================================
// Threshold
const int LOW_THRESHOLD                     = 30;
const int WARNING_LOW_THRESHOLD             = 40;
const int WARNING_HIGH_THRESHOLD            = 80;
const int HIGH_THRESHOLD                    = 90;

// Loop
const int INTERVAL_CHECK                    = 5000; // 5s
const int INTERVAL_CHECK_WHEN_WARNING       = 1000; // 1s

// Nofication
const bool ACTIVE_NOFICATION                = true;
const int REPEAT_NOFICATION_AFTER_PERCENT   = 5;

// Popup
const bool ACTIVE_POPUP                     = true;
const float POPUP_SCALE                     = 1;

// Other
const bool DEBUGING                         = true;







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

void delay(std::string warningState){
    if (warningState != "") {
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_CHECK_WHEN_WARNING));
    }
    else {
        std::this_thread::sleep_for(std::chrono::milliseconds(INTERVAL_CHECK));
    }
}

std::string getTitle(std::string warningState){
    if (warningState == "L")  return "Battery too low";
    if (warningState == "WL") return "Low battery";
    if (warningState == "WH") return "High battery";
    if (warningState == "H") return  "Battery too high";
    return "";
}

std::string getDesc(std::string warningState){
    if (warningState == "L" || warningState == "WL") return "Plug in now!";
    if (warningState == "H" || warningState == "WH") return "Unplug now!";
    return "";
}




// ======================================================================================================================
// MAIN
// ======================================================================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{

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


        // Loop
        while (true) {
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            std::cout << "Check...\n";
            

            // Get data
            {
                prevPercent = percent;
                percent = Battery::GetBatteryPercent();
                charging = Battery::IsCharging();

                if (DEBUGING) {
                    std::cout << "[DEBUG] Input percent: ";
                    std::cin >> percent;
                    std::cout << "[DEBUG] Input is charging (y/n): ";
                    std::string inp;
                    std::cin >> inp;
                    if (inp == "y") charging = true; 
                    if (inp == "n") charging = false; 
                }

                prevWarningState = warningState;
                warningState = checkWarning(percent, charging);                
            }

            
            // Notification
            {
                if (ACTIVE_NOFICATION){
                    if (warningState != ""){
                        if (warningState != prevWarningState || abs(percent - prevNotificationPercent) >= REPEAT_NOFICATION_AFTER_PERCENT) {
                            std::string title = getTitle(warningState) + " (" + std::to_string(percent) + "%)";
                            Notification::Show(title, getDesc(warningState));
                            prevNotificationPercent = percent;
                        }
                    }                    
                }
            } 

            // Popup
            {
                if (ACTIVE_POPUP){
                    if (warningState != ""){
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
            }
            

            std::cout << "Result: " << warningState << "\n\n";
            // Delay
            delay(warningState);
        }
    }
    else {
        std::cout << "No battery found!";
    }


    return 0;
}


