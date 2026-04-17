#pragma once
#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

class WarningIcon {
public:
    static void Init();
    static void Shutdown();

    // Load từ resource ID (PNG embedded trong exe)
    static Gdiplus::Image* LoadFromResource(int resourceId = 101);

    // Vẽ icon
    static void Draw(HDC hdc, int x, int y, int w, int h, Gdiplus::Image* img);

private:
    static ULONG_PTR gdiToken;
};
