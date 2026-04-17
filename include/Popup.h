#pragma once
#include <windows.h>
#include <string>

namespace Popup
{
    // ===== COLOR CONFIG =====
    inline const int LOW_COLOR[3] = { 255, 0, 0 };
    inline const int WARNING_COLOR[3] = { 255, 255, 0 };
    inline const int GOOD_COLOR[3] = { 0, 255, 0 };

    // ===== API =====
    void Init(HINSTANCE hInstance, float scale = 1.0f);
    void Shutdown();

    void Show(int percent,
        const std::string& title,
        const std::string& desc,
        int lt, int wlt, int wht, int ht);

    void Hide();
}