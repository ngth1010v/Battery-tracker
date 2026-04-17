#include "batteryApi.h"

namespace Battery
{
    bool HasBattery()
    {
        SYSTEM_POWER_STATUS status;

        if (!GetSystemPowerStatus(&status))
            return false;

        // 128 = no battery
        if (status.BatteryFlag == 128 || status.BatteryFlag == 255)
            return false;

        return true;
    }

    int GetBatteryPercent()
    {
        SYSTEM_POWER_STATUS status;

        if (!GetSystemPowerStatus(&status))
            return 0;

        // không có pin
        if (status.BatteryFlag == 128 || status.BatteryFlag == 255)
            return 0;

        return (int)status.BatteryLifePercent;
    }

    bool IsCharging()
    {
        SYSTEM_POWER_STATUS status;

        if (!GetSystemPowerStatus(&status))
            return false;

        // ACLineStatus:
        // 0 = offline (không cắm sạc)
        // 1 = online (đang cắm sạc)
        // 255 = unknown
        if (status.ACLineStatus == 1)
            return true;

        return false;
    }
}