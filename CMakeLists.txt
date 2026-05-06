cmake_minimum_required(VERSION 4.2)
project(Chip8Att2)
set(CMAKE_CXX_STANDARD 26)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE) # enable wayland on linux

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(Chip8Att2 main.cpp
        cores/chip8/chip8.cpp
        cores/chip8/chip8.h
        external/portable-file-dialogs/portable-file-dialogs.h
        ui/ui.cpp
        ui/ui.h
        cores/schip/schip.h
        cores/schip/schip.cpp
        app.h)

target_include_directories(Chip8Att2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Chip8Att2 PRIVATE raylib)