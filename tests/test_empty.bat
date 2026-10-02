@echo off
cd /d "%~dp0.."

"x64\Debug\emulator.exe" "%~dp0..\vfs" "%~dp0empty.txt"

echo Exit code: %ERRORLEVEL%
pause
