@echo off
REM Create build directory if it doesn't exist
if not exist build (
    mkdir build
)

REM Run CMake config if it's not already generated
if not exist build/CMakeCache.txt (
    cmake -S . -B build
)

REM Build the project
cmake --build build

REM Run the compiled executable
build\Debug\FlintInterpreter.exe

pause