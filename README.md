# 🔋 BatteryTracker

A lightweight Windows application that monitors battery status and shows notifications based on charging conditions.

## ✨ Features

- Real-time battery percentage tracking
- Charging / discharging detection
- Custom warning notifications
- Lightweight and minimal resource usage
- Runs silently in background

## 🚀 Installation

### Option 1: Prebuilt Release
1. Download the latest release
2. Run `BatteryTracker_Setup.exe`
3. Done ✔

### Option 2: Build from source

Requirements:
- C++ compiler (GCC / MSYS2 or MSVC)
- CMake

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release

If Windows shows "protected your PC":
Click "More info" → "Run anyway"