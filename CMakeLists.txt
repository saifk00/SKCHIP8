cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# SDL2
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG b424665e0899769b200231ba943353a5fee1b6b6 # v2.0.20
)
FetchContent_MakeAvailable(SDL2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SKCHIP8)

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
target_link_libraries(SKChip8Core SKChip8Utils)

# Chip-8 emulator library
set(CHIP8_EMULATOR_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/chip-8-emulator)
add_library(SKChip8Emulator
    ${CHIP8_EMULATOR_SRC_DIR}/Emulator.cpp)

target_link_libraries(SKChip8Emulator SKChip8Core)
target_include_directories(SKChip8Emulator PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/SKChip8/Emulator"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/SKChip8/")

# Main Application (SDL)
set(CHIP8_SDL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/emu)
add_executable(SKChip8SDL WIN32
    ${CHIP8_SDL_SRC_DIR}/main.cpp
    ${CHIP8_SDL_SRC_DIR}/SDLEmuAdapter.cpp)

target_link_libraries(SKChip8SDL
    SDL2
    SKChip8Emulator)

target_include_directories(SKChip8SDL 
    PRIVATE "include")