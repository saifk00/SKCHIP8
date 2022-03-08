#ifndef _DEBUG_WINDOW_H
#define _DEBUG_WINDOW_H

#include <Utils/CHIP8Utils.h>
#include <Utils/CHIP8ISA.h>

#include <vector>
#include <cstdint>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"

#include <SDL.h>

#include "SDLEmuAdapter.h"

class DebuggingWindow
{
public:
    DebuggingWindow(std::shared_ptr<SDLEmuAdapter> emulator);
    void Update();
    Uint32 GetWindowID() const { return SDL_GetWindowID(window_); }
    void HandleEvent(const SDL_Event &event);

protected:
    void initializeWindow();
    void destroyWindow();
    void drawStateInfoPane();
    void drawMemoryPane();
    void drawEmulatorInfoPane();
    void drawControlPane();

private:
    std::shared_ptr<SDLEmuAdapter> emulator_;
    SDL_Window *window_;
    SDL_GLContext glContext_;
    ImGuiContext *imGuiContext_;
    bool display_;
};

DebuggingWindow::DebuggingWindow(std::shared_ptr<SDLEmuAdapter> emulator)
{
    emulator_ = emulator;
    display_ = true;
    initializeWindow();
}

void DebuggingWindow::HandleEvent(const SDL_Event &event)
{
    if (!display_)
        return;

    ImGui_ImplSDL2_ProcessEvent(&event);

    // handle events
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_CLOSE)
    {
        destroyWindow();
    }
}

void DebuggingWindow::Update()
{
    if (!display_)
        return;

    // init
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window_);
    ImGui::NewFrame();

    // draw panes
    {
        drawStateInfoPane();
        drawMemoryPane();
        drawControlPane();
        drawEmulatorInfoPane();
    }

    // render it
    ImGui::Render();
    auto &io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.1f, 0.05f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window_);
}

void DebuggingWindow::drawStateInfoPane()
{
    static int keymap[] = {
        1, 2, 3, 0xC,
        4, 5, 6, 0xD,
        7, 8, 9, 0xE,
        0xA, 0, 0xB, 0xF};

    ImGui::Begin("State Info");
    const auto &cpu = emulator_->GetCPU();
    std::stringstream instructionStringStream;
    SKChip8::DecodeInstruction(cpu->GetCurrentInstruction())->dump(instructionStringStream);

    ImGui::Text("PC: %04X", cpu->GetPC(), instructionStringStream.str().c_str());
    ImGui::TextColored(ImVec4(0.090f, 0.929f, 0.933f, 1.0f), "%s\n\n", instructionStringStream.str().c_str());

    ImGui::Text("I: %04X", cpu->GetIndexPointer());
    // TODO(sk00): add address register, maybe the 5-byte sprite pointed to by I, and timer values

    // print registers inline
    auto registers = cpu->GetRegisters();
    ImGui::Text("V0: %02X V1: %02X V2: %02X V3: %02X V4: %02X V5: %02X V6: %02X V7: %02X",
                registers[0], registers[1], registers[2], registers[3], registers[4], registers[5], registers[6], registers[7]);
    ImGui::Text("V8: %02X V9: %02X VA: %02X VB: %02X VC: %02X VD: %02X VE: %02X VF: %02X",
                registers[8], registers[9], registers[10], registers[11], registers[12], registers[13], registers[14], registers[15]);

    // print timers
    auto delaytimer = cpu->GetDelayTimer();
    auto soundtimer = cpu->GetSoundTimer();
    ImGui::Text("Delay Timer: %02X", delaytimer);
    ImGui::SameLine();
    ImGui::Text("Sound Timer: %02X", soundtimer);

    // print keyboard state
    auto keyboardState = cpu->GetKeyState();
    ImGui::Text("Keyboard State:");
    std::stringstream keyboardStateStream;
    for (int i = 0; i < 16; ++i)
    {
        keyboardStateStream << (keyboardState[keymap[i]] ? "1" : "0");
        if (i % 4 == 3)
            keyboardStateStream << "\n";
    }
    ImGui::Text("%s", keyboardStateStream.str().c_str());

    ImGui::End();
}

void DebuggingWindow::drawMemoryPane()
{
    static MemoryEditor memoryEditor;
    memoryEditor.ReadOnly = true;

    const auto &cpu = emulator_->GetCPU();
    auto memory = cpu->GetMemory();

    memoryEditor.DrawWindow("Memory Editor", memory.data(), memory.size());
}

void DebuggingWindow::drawControlPane()
{
    ImGui::Begin("Controls");

    if (ImGui::Button("Stop"))
    {
        emulator_->Disable();
    }

    if (ImGui::Button("Start"))
    {
        emulator_->Enable();
    }

    if (ImGui::Button("Step"))
    {
        emulator_->Step();
    }

    if (ImGui::Button("Reset"))
    {
        emulator_->Reset();
    }

    ImGui::End();
}

void DebuggingWindow::drawEmulatorInfoPane()
{
    ImGui::Begin("Emulator Info");

    ImGui::Text("Instructions Per Tick: %d", emulator_->GetIPT());
    ImGui::Text("Frames Per Second: %f", emulator_->GetFPS());
    ImGui::Text("Instructions Per Frame: %f", emulator_->GetInstructionsPerFrame());

    ImGui::End();
}

void DebuggingWindow::initializeWindow()
{
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window_ = SDL_CreateWindow("Chip-8 Debugging", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    glContext_ = SDL_GL_CreateContext(window_);

    // initialize OpenGL context
    // note that this has to be done after the initial glcontext is created
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        throw std::runtime_error("Failed to initialize OpenGL context");
    }

    SDL_GL_MakeCurrent(window_, glContext_);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    imGuiContext_ = ImGui::CreateContext();
    auto &io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window_, glContext_);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void DebuggingWindow::destroyWindow()
{
    ImGui::SetCurrentContext(imGuiContext_);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(glContext_);
    SDL_DestroyWindow(window_);

    display_ = false;
}

#endif