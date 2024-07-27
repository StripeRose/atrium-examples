@echo off

if %errorlevel% EQU 0 (
    "./atrium/tools/Sharpmake/Sharpmake.Application/bin/Release/net6.0/Sharpmake.Application.exe" "/sources('chart-game/sharpmake.cs')"
)
