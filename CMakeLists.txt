cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SKCHIP8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)
include_directories("${SDL2_INCLUDE_DIR}")

find_package(Threads REQUIRED)

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

# Main Application (SDL)
set(CHIP8_SDL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/emu-sdl)
add_executable(SKChip8SDL WIN32
    ${CHIP8_SDL_SRC_DIR}/main.cpp
    ${CHIP8_SDL_SRC_DIR}/SDLEmuAdapter.cpp)

target_link_libraries(SKChip8SDL
    "${SDL2_LIBRARY}"
    SKChip8Emulator)

target_include_directories(SKChip8SDL 
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

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