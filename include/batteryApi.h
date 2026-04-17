#pragma once
#include <windows.h>

namespace Battery
{
    // true = có pin / false = không có pin
    // NOTE: unknown vẫn tính là có pin (theo yêu cầu của bạn)
    bool HasBattery();

    // trả về % pin (0 - 100)
    // nếu không có pin → trả 0
    int GetBatteryPercent();

    // true = đang cắm sạc / false = không cắm sạc hoặc không xác định
    bool IsCharging();
}