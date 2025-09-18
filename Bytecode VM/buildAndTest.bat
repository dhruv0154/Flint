@echo off
REM ==============================
REM Build and Run Flint with MinGW
REM ==============================

REM Change directory to the script’s location
cd /d "%~dp0"

REM Create build directory if it doesn’t exist
if not exist build (
    mkdir build
)

REM Configure project with MinGW Makefiles
cmake -S . -B build -G "Ninja" -DCMAKE_CXX_COMPILER=g++

REM Build project
cmake --build build

REM Run the interpreter with test.flint
build\flint.exe
