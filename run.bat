@echo off
setlocal

echo ================================
echo   BatteryTracker - RUN
echo ================================

cd /d %~dp0

REM Move into build folder
cd build

REM Run EXE
if exist BatteryTracker.exe (
echo Running BatteryTracker.exe ...
cls
BatteryTracker.exe
) else (
echo [ERROR] BatteryTracker.exe not found!
echo Please run build.bat first.
)

echo.
pause
endlocal
