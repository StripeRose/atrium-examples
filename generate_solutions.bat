@echo off

cd "./rose-gold/"
call "./setup_repo.bat"
cd ..

if %errorlevel% EQU 0 (
    "./rose-gold/tools/Sharpmake/Sharpmake.Application/bin/Release/net6.0/Sharpmake.Application.exe" "/sources('chart-game/sharpmake.cs')"
)
