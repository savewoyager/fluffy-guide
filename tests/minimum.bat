@echo off
cd /d "%~dp0.."
"x64\Debug\emulator.exe" "vfs\minimum.xml" "tests\startup.txt"
pause
