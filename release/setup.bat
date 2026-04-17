@echo off
setlocal enabledelayedexpansion

:: ===== 1. FIND EXE =====
set "EXE_PATH="

for /r %%i in (BatteryTracker.exe) do (
    set "EXE_PATH=%%i"
    goto :found
)

:found
if not defined EXE_PATH (
    echo [ERROR] Cannot find BatteryTracker.exe
    timeout /t 3 >nul
    exit /b
)

:: ===== 2. ADD TO STARTUP (REGISTRY) =====
reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" ^
/v "BatteryTracker" ^
/t REG_SZ ^
/d "!EXE_PATH!" ^
/f >nul 2>&1

if errorlevel 1 (
    echo [ERROR] Failed to add to startup
    timeout /t 3 >nul
    exit /b
)

:: ===== 3. RUN APP =====
start "" "!EXE_PATH!"

:: ===== 4. EXIT SILENT =====
exit