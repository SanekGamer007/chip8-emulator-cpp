cmake_minimum_required(VERSION 4.2)
project(Chip8Att2)

set(CMAKE_CXX_STANDARD 26)

find_package(raylib REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_executable(Chip8Att2 main.cpp
        chip8/chip8.cpp
        chip8/chip8.h
        portable-file-dialogs.h)

target_link_libraries(Chip8Att2 PRIVATE raylib)