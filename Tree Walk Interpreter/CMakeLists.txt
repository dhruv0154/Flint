cmake_minimum_required(VERSION 3.15)
project(FlintInterpreter LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

# Build the executable
add_executable(flint ${SRC_FILES})

target_include_directories(flint PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)