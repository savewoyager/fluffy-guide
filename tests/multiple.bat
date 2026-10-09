@echo off
cd /d "%~dp0.."
"x64\Debug\emulator.exe" "vfs\multiple.xml" "tests\startup.txt"
pause