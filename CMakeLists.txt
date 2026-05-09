cmake_minimum_required(VERSION 4.2)
project(Chip8Att2)
set(CMAKE_CXX_STANDARD 26)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
set(BUILD_RAYGUI_EXAMPLES OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE) # enable wayland on linux

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(USE_EXTERNAL_GLFW OFF CACHE BOOL "" FORCE)
    set(PLATFORM "Desktop" CACHE STRING "" FORCE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM "SDL" CACHE STRING "" FORCE)
endif()


FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "6.0"
        GIT_PROGRESS TRUE
)
FetchContent_Declare(
        portable_file_dialogs
        GIT_REPOSITORY "https://github.com/samhocevar/portable-file-dialogs.git"
        GIT_TAG "main"
        GIT_PROGRESS TRUE
)
FetchContent_Declare(
        raygui
        GIT_REPOSITORY "https://github.com/raysan5/raygui.git"
        GIT_TAG "4.0"
        GIT_PROGRESS TRUE
        SOURCE_SUBDIR "projects/CMake"
)

FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(raygui)
FetchContent_MakeAvailable(portable_file_dialogs)

add_executable(Chip8Att2 main.cpp
        cores/chip8/chip8.cpp
        cores/chip8/chip8.h
        ui/ui.cpp
        ui/ui.h
        cores/schip/schip.h
        cores/schip/schip.cpp
        app.h
        config.h)

target_include_directories(Chip8Att2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_options(Chip8Att2 PRIVATE -static-libgcc -static-libstdc++ -static)
endif()
target_link_libraries(Chip8Att2 PRIVATE raylib raygui portable_file_dialogs)