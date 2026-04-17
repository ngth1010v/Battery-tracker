#include "WarningIcon.h"



ULONG_PTR WarningIcon::gdiToken = 0;

void WarningIcon::Init() {
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&gdiToken, &input, nullptr);
}

void WarningIcon::Shutdown() {
    if (gdiToken) {
        Gdiplus::GdiplusShutdown(gdiToken);
        gdiToken = 0;
    }
}

Gdiplus::Image* WarningIcon::LoadFromResource(int resourceId)
{
    HRSRC hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(resourceId), L"PNG");
    if (!hRes) return nullptr;

    HGLOBAL hData = LoadResource(nullptr, hRes);
    if (!hData) return nullptr;

    void* pData = LockResource(hData);
    DWORD size = SizeofResource(nullptr, hRes);

    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hBuffer) return nullptr;

    void* pBuffer = GlobalLock(hBuffer);
    memcpy(pBuffer, pData, size);
    GlobalUnlock(hBuffer);

    IStream* stream = nullptr;
    CreateStreamOnHGlobal(hBuffer, TRUE, &stream);

    Gdiplus::Image* img = new Gdiplus::Image(stream);

    stream->Release();
    return img;
}

void WarningIcon::Draw(HDC hdc, int x, int y, int w, int h, Gdiplus::Image* img)
{
    if (!img) return;

    Gdiplus::Graphics g(hdc);
    g.DrawImage(img, x, y, w, h);
}
