cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# SDDL
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG b424665e0899769b200231ba943353a5fee1b6b6 # v2.0.20
)
FetchContent_MakeAvailable(SDL2)

project(SKCHIP8)

# Chip-8 core library
set(CHIP8_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/chip-8-core)
add_library(SKChip8 
    ${CHIP8_SRC_DIR}/CPU.cpp 
    ${CHIP8_SRC_DIR}/Memory.cpp
    ${CHIP8_SRC_DIR}/utils/ROMLoader.cpp
    ${CHIP8_SRC_DIR}/utils/CHIP8Utils.cpp)

target_include_directories(SKChip8 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/SKChip8")

# Main Application
set(CHIP8_EMU_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/emu)
add_executable(SKChip8Emu WIN32
    ${CHIP8_EMU_SRC_DIR}/main.cpp)

target_link_libraries(SKChip8Emu
    SDL2
    SKChip8)

target_include_directories(SKChip8Emu
    PRIVATE "include")