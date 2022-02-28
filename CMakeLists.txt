cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SKCHIP8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)
include_directories("${SDL2_INCLUDE_DIR}")

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

# Chip-8 utils library
set(CHIP8_UTILS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/chip-8-utils)
add_library(SKChip8Utils
    ${CHIP8_UTILS_SRC_DIR}/CHIP8ISA.cpp
    ${CHIP8_UTILS_SRC_DIR}/ROMLoader.cpp)

target_include_directories(SKChip8Utils PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/SKChip8/Utils")


# Chip-8 core library
set(CHIP8_CORE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/chip-8-core)
add_library(SKChip8Core
    ${CHIP8_CORE_SRC_DIR}/CPU.cpp)

    # TODO(sk00) how to make this private
target_include_directories(SKChip8Core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/SKChip8/Core"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/SKChip8/")
target_link_libraries(SKChip8Core SKChip8Utils Threads::Threads)

# Chip-8 emulator library
set(CHIP8_EMULATOR_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/chip-8-emulator)
add_library(SKChip8Emulator
    ${CHIP8_EMULATOR_SRC_DIR}/Emulator.cpp)

target_link_libraries(SKChip8Emulator SKChip8Core)
target_include_directories(SKChip8Emulator PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/SKChip8/Emulator"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/SKChip8/")

# ImGUI library
set(IMGUI_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)
add_library(ImGUI
    ${IMGUI_SRC_DIR}/imgui.cpp
    ${IMGUI_SRC_DIR}/imgui_draw.cpp
    ${IMGUI_SRC_DIR}/imgui_widgets.cpp
    ${IMGUI_SRC_DIR}/imgui_tables.cpp
    ${IMGUI_SRC_DIR}/imgui_demo.cpp
    ${IMGUI_SRC_DIR}/backends/imgui_impl_sdl.cpp
    ${IMGUI_SRC_DIR}/backends/imgui_impl_sdlrenderer.cpp
    ${IMGUI_SRC_DIR}/backends/imgui_impl_opengl3.cpp)
set(IMGUI_INCLUDE_DIRS ${IMGUI_SRC_DIR} ${IMGUI_SRC_DIR}/backends)

target_include_directories(ImGUI PUBLIC "${IMGUI_INCLUDE_DIRS}")

# ImGUI extensions
set(IMGUI_EXTENSIONS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui_club/imgui_memory_editor)

# GLAD library
set(GLAD_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/src)
set(GLAD_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/include)
add_library(GLAD ${GLAD_SRC_DIR}/glad.c)
target_include_directories(GLAD PUBLIC "${GLAD_INC_DIR}")

# Main Application (SDL)
set(CHIP8_SDL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/emu-sdl)
add_executable(SKChip8SDL WIN32
    ${CHIP8_SDL_SRC_DIR}/main.cpp
    ${CHIP8_SDL_SRC_DIR}/SDLEmuAdapter.cpp
    ${CHIP8_SDL_SRC_DIR}/DebuggingWindow.hpp
    ${CHIP8_SDL_SRC_DIR}/EmulatorWindow.hpp)

target_link_libraries(SKChip8SDL
    GLAD
    "${OPENGL_LIBRARIES}"
    ImGUI
    "${SDL2_LIBRARY}"
    SKChip8Emulator)

target_include_directories(SKChip8SDL 
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    ${IMGUI_INCLUDE_DIRS}
    ${IMGUI_EXTENSIONS_INCLUDE_DIRS}
    ${GLAD_INC_DIR})

# opengl/sdl/imgui test application
add_executable(SKChip8SDLTest WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gltest.cpp)
target_link_libraries(SKChip8SDLTest
    GLAD
    "${OPENGL_LIBRARIES}"
    ImGUI
    "${SDL2_LIBRARY}")
target_include_directories(SKChip8SDLTest
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    ${IMGUI_INCLUDE_DIRS}
    ${GLAD_INC_DIR})

# Chip 8 Emulator CLI mode
set(CHIP8_CLI_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/emu-cli)
add_executable(SKChip8EmuCLI
    ${CHIP8_CLI_SRC_DIR}/main.cpp)

target_link_libraries(SKChip8EmuCLI
    SKChip8Emulator)

target_include_directories(SKChip8EmuCLI 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Chip 8 Disassembler
set(CHIP8_DUMP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/chip8dump)
add_executable(Chip8Dump
	${CHIP8_DUMP_SRC_DIR}/main.cpp)

target_link_libraries(Chip8Dump
	SKChip8Utils)

target_include_directories(Chip8Dump
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")