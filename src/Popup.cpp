#include "Popup.h"
#include "WarningIcon.h"
#include <gdiplus.h>
#include <iostream>
#include <algorithm>

#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;

std::wstring ToWString(const std::string& s)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    std::wstring ws(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], size);
    ws.pop_back();
    return ws;
}

namespace Popup
{
    // ================= GLOBAL =================
    HWND g_hwnd = NULL;

    int g_percent = 0;
    std::wstring g_title;
    std::wstring g_desc;

    int g_lt, g_wlt, g_wht, g_ht;

    Gdiplus::Image* g_icon = nullptr;

    float g_scale = 1.0f;

    // ================= SCALE =================
    int S(int v) {
        return (int)(v * g_scale);
    }

    float ClampScale(float s) {
        return std::max(0.5f, std::min(100.0f, s));
    }

    // ================= UTILS =================
    COLORREF ToColor(const int c[3]) {
        return RGB(c[0], c[1], c[2]);
    }

    COLORREF BlendColor(COLORREF color, float alpha) {
        int r = GetRValue(color);
        int g = GetGValue(color);
        int b = GetBValue(color);

        int br = 37, bg = 37, bb = 34;

        int nr = (int)(r * alpha + br * (1 - alpha));
        int ng = (int)(g * alpha + bg * (1 - alpha));
        int nb = (int)(b * alpha + bb * (1 - alpha));

        return RGB(nr, ng, nb);
    }

    void DrawRoundedRect(HDC hdc, RECT rect, int radius, HBRUSH brush, HPEN pen) {
        auto oldBrush = (HBRUSH)SelectObject(hdc, brush);
        auto oldPen = (HPEN)SelectObject(hdc, pen);

        RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
    }

    // ================= DRAW =================
    void DrawUI(HDC hdc, RECT rect)
    {
        int width = rect.right;
        int height = rect.bottom;

        // ===== bg =====
        HBRUSH bg = CreateSolidBrush(RGB(37, 37, 34));
        HPEN border = CreatePen(PS_SOLID, S(2), RGB(136, 136, 68));
        DrawRoundedRect(hdc, rect, 0, bg, border);
        DeleteObject(bg);
        DeleteObject(border);

        // ===== icon =====
        Graphics g(hdc);
        if (g_icon)
            g.DrawImage(g_icon, S(10), S(8), S(60), S(60));

        SetBkMode(hdc, TRANSPARENT);

        // ===== font =====
        HFONT fMain = CreateFont(
            S(20), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas");

        HFONT fSub = CreateFont(
            S(13), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH, L"Consolas");

        int textX = S(80);

        // title
        SelectObject(hdc, fMain);
        SetTextColor(hdc, RGB(255, 255, 136));
        TextOut(hdc, textX, S(8), g_title.c_str(), g_title.size());

        // desc
        SelectObject(hdc, fSub);
        SetTextColor(hdc, RGB(187, 187, 0));
        TextOut(hdc, textX, S(30), g_desc.c_str(), g_desc.size());

        DeleteObject(fMain);
        DeleteObject(fSub);

        // ===== progress =====
        int barX = textX;
        int barY = height - S(18);
        int barW = width - textX - S(10);
        int barH = S(7);

        struct Segment {
            int from;
            int to;
            COLORREF color;
            bool highlight;
        };

        Segment segs[] = {
            {0, g_lt, ToColor(LOW_COLOR), false},
            {g_lt, g_wlt, ToColor(WARNING_COLOR), false},
            {g_wlt, g_wht, ToColor(GOOD_COLOR), false},
            {g_wht, g_ht, ToColor(WARNING_COLOR), false},
            {g_ht, 100, ToColor(LOW_COLOR), false},
        };

        for (auto& s : segs) {
            if (g_percent >= s.from && g_percent < s.to)
                s.highlight = true;
        }

        int curX = barX;

        for (auto& s : segs)
        {
            int w = barW * (s.to - s.from) / 100;

            RECT r = { curX, barY, curX + w, barY + barH };

            COLORREF c = s.highlight ? s.color : BlendColor(s.color, 0.4f);

            HBRUSH b = CreateSolidBrush(c);
            FillRect(hdc, &r, b);
            DeleteObject(b);

            curX += w;
        }

        // navigator
        int navX = barX + barW * g_percent / 100;

        HPEN pen = CreatePen(PS_SOLID, S(2), RGB(255, 255, 170));
        SelectObject(hdc, pen);

        MoveToEx(hdc, navX, barY - S(16), NULL);
        LineTo(hdc, navX, barY + barH);

        DeleteObject(pen);

        std::wstring txt = std::to_wstring(g_percent) + L"%";
        SetTextColor(hdc, RGB(255, 255, 200));

        if (g_percent >= 80)
            TextOut(hdc, navX - S(25), barY - S(16), txt.c_str(), txt.size());
        else
            TextOut(hdc, navX + S(3), barY - S(16), txt.c_str(), txt.size());
    }

    // ================= WINDOW =================
    LRESULT CALLBACK Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_NCHITTEST: return HTTRANSPARENT;
        case WM_MOUSEACTIVATE: return MA_NOACTIVATE;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT r;
            GetClientRect(hwnd, &r);
            DrawUI(hdc, r);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CLOSE:
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    // ================= API =================
    void Init(HINSTANCE hInstance, float scale)
    {
        g_scale = ClampScale(scale);

        WarningIcon::Init();
        g_icon = WarningIcon::LoadFromResource(101);

        WNDCLASS wc = {};
        wc.lpfnWndProc = Proc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"PopupWindow";

        RegisterClass(&wc);

        int w = S(300);
        int h = S(80);

        g_hwnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            wc.lpszClassName,
            L"",
            WS_POPUP,
            0, 0, w, h,
            NULL, NULL, hInstance, NULL
        );
    }

    void Show(int percent,
        const std::string& title,
        const std::string& desc,
        int lt, int wlt, int wht, int ht)
    {
        g_percent = percent;
        g_title = ToWString(title);
        g_desc = ToWString(desc);

        g_lt = lt;
        g_wlt = wlt;
        g_wht = wht;
        g_ht = ht;

        RECT wa;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0);

        int w = S(300);
        int h = S(80);

        int x = wa.right - w - S(10);
        int y = wa.bottom - h - S(10);

        SetWindowPos(g_hwnd, HWND_TOPMOST,
            x, y, w, h,
            SWP_SHOWWINDOW | SWP_NOACTIVATE);

        InvalidateRect(g_hwnd, NULL, TRUE);
    }

    void Hide()
    {
        if (g_hwnd)
            ShowWindow(g_hwnd, SW_HIDE);
    }

    void Shutdown()
    {
        if (g_icon) {
            delete g_icon;
            g_icon = nullptr;
        }

        WarningIcon::Shutdown();
    }
}