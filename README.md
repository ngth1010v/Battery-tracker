# 🔋 Battery Tracker

![License](https://img.shields.io/badge/license-Unlicense-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![C++](https://img.shields.io/badge/language-C%2B%2B-00599C.svg)

> Lightweight background app to monitor battery percentage and help extend Li-ion battery lifespan.

---

## ⚠️ Requirements

* 💻 **Laptop only** (requires battery hardware)
* 🪟 Windows OS

---

## ✨ Features

* 🔔 **Smart battery alerts**

  * Notify when battery drops below **40%** and not plugged in.
  * Notify when battery exceeds **80%** and still plugged in.

* 🪟 **Dual notification system**

  * Native Windows notification
  * Custom popup at bottom-right corner

* ⚡ **Lightweight & silent**

  * Runs in background
  * No taskbar icon (main UI hidden)
  * Minimal resource usage

* 🚀 **Auto start with Windows**

* 🧭 **System tray control**

  * App runs with a **tray icon on taskbar system tray**
  * Allows **quick toggle ON / OFF tracker**
  * Right-click / interaction from tray for fast control

* ⚙️ **Configurable via config.ini**

  * App supports external configuration file: `config.ini`
  * File is located **next to the BatteryTracker.exe file**
  * Allows users to modify thresholds and behavior without recompiling

  ### 🛠️ How to edit config

  * Open `config.ini`
  * Edit values as needed
  * Save file
  * Then apply changes by:

    * Restarting tracker from system tray **OR**
    * Restarting the computer

---

## 📸 Preview

> Popup notification example:

![Popup Preview](./assets/popup.jpg)

---

## 📦 Installation

1. Go to **Releases**
2. Download latest `.exe`
3. Run installer
4. Done — app will run automatically every time you open Windows

---

## 🛡️ Windows Defender Warning

> ⚠️ This app may be flagged by Windows Defender as an unknown application.

This happens because:

* The app is **not code signed**
* It is a **new / low-distribution executable**

### ✅ How to run anyway

1. When SmartScreen appears → click **"More info"**
2. Click **"Run anyway"**

### 🔒 Safety note

* This project is **fully open-source**
* You can **build it yourself from source**
* No hidden or malicious behavior

---

## 🚀 Usage

* No configuration needed (optional config via `config.ini`)
* App runs silently in background
* Automatically monitors battery status

---

## 🛠️ Build from source

### Requirements

* MSYS2 (UCRT64)
* CMake
* C++17 compatible compiler

### Build steps

```bash
git clone https://github.com/ngth1010v/Battery-tracker.git
cd battery-tracker
build.bat
```

---

## 📌 Notes

* Designed specifically for **Li-ion battery health optimization**
* Recommended usage range: **40% – 80%**
* Tray + config system added for better control and customization

---

## 📜 License

This project is released under the **Unlicense** — free to use, modify, and distribute without restriction.

---

## ⭐ Support

If you find this project useful, consider giving it a star on GitHub ⭐
