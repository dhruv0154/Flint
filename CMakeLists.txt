cmake_minimum_required(VERSION 3.10)
project(FlintInterpreter)

set(CMAKE_CXX_STANDARD 17)

# Include all header directories
include_directories(include)

# Automatically find all .cpp files in src/, excluding tool/
file(GLOB_RECURSE ALL_SOURCES src/**/*.cpp)

# Filter out any file paths that contain "/tool/"
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*/Tool/.*")

# Create the executable
add_executable(FlintInterpreter ${ALL_SOURCES})