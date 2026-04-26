cmake_minimum_required(VERSION 4.2)
project(Chip8Att2)

set(CMAKE_CXX_STANDARD 26)

find_package(raylib REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)


add_executable(Chip8Att2 main.cpp
        cores/chip8/chip8.cpp
        cores/chip8/chip8.h
        external/portable-file-dialogs/portable-file-dialogs.h
        ui/ui.cpp
        ui/ui.h
        cores/schip/schip.h
        cores/schip/schip.cpp)

target_include_directories(Chip8Att2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Chip8Att2 PRIVATE raylib)