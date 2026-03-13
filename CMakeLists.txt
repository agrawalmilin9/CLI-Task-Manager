cmake_minimum_required(VERSION 3.16)
project(TaskManager CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
add_compile_options(
    -Wall -Wextra -Wpedantic
    -O2
)

include_directories(include)

add_executable(task_manager
    src/main.cpp
    src/Task.cpp
    src/TaskManager.cpp
    src/CLI.cpp
)

# Install target
install(TARGETS task_manager DESTINATION bin)
