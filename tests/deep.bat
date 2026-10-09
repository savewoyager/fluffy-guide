@echo off
cd /d "%~dp0.."
"x64\Debug\emulator.exe" "vfs\deep.xml" "tests\startup.txt"
pause