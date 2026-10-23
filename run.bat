@echo off
chcp 65001 >nul
REM run.bat — сборка, запуск и тестирование my-shell под Windows
REM Использование:
REM   run.bat        — собрать и запустить программу
REM   run.bat test   — собрать и прогнать тесты

setlocal

set TARGET=my-shell.exe
set SRC=src\emulator.cpp
set CXX=g++
set CXXFLAGS=-std=c++17 -Wall -Wextra -Wpedantic -O2

where %CXX% >nul 2>nul
if errorlevel 1 (
    echo [ERROR] g++ not found in PATH.
    echo Install w64devkit / MSYS2 / MinGW-w64 and add its bin to PATH.
    exit /b 1
)

echo [BUILD] Compiling %SRC% ...
%CXX% %CXXFLAGS% %SRC% -o %TARGET%
if errorlevel 1 (
    echo [BUILD] Compilation failed.
    exit /b 1
)
echo [BUILD] Done: %TARGET%
echo.

if /I "%~1"=="test" goto :test

echo [RUN] Running %TARGET% ...
echo.
%TARGET%
exit /b %errorlevel%

:test
echo [TEST] Running tests ...
echo.
%TARGET% < tests\test_input.txt > tests\actual_output.txt 2>&1
fc /N tests\expected_output.txt tests\actual_output.txt >nul
if errorlevel 1 (
    echo [FAIL] Output differs from expected.
    fc /N tests\expected_output.txt tests\actual_output.txt
    exit /b 1
)
echo [PASS] All tests passed.
del tests\actual_output.txt
exit /b 0