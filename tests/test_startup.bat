@echo off
cd /d "%~dp0.."

"x64\Debug\emulator.exe" "vfs\base.xml" "tests\startup.txt"

pause
