cmake_minimum_required(VERSION 3.10)
project(FlintInterpreter)

set(CMAKE_CXX_STANDARD 17)

# Include all header directories
include_directories(include)

# Automatically find all .cpp files in src/
file(GLOB_RECURSE SOURCES src/**/*.cpp)

# Create the executable
add_executable(FlintInterpreter ${SOURCES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)