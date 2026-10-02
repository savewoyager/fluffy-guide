@echo off
cd /d "%~dp0.."

"x64\Debug\emulator.exe"

echo Exit code: %ERRORLEVEL%
pause
