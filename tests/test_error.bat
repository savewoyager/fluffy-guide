@echo off
cd /d "%~dp0"

"x64\Debug\emulator.exe" "%~dp0vfs" "%~dp0missing.txt"

echo Exit code: %ERRORLEVEL% file not found \('_')/

pause