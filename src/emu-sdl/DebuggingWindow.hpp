#ifndef _DEBUG_WINDOW_H
#define _DEBUG_WINDOW_H

#include <vector>
#include <cstdint>
#include <memory>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

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

private:
    std::shared_ptr<SDLEmuAdapter> Emulator_;
    SDL_Window *window_;
    SDL_GLContext glContext_;
    ImGuiContext *imGuiContext_;
    bool display_;
};

DebuggingWindow::DebuggingWindow(std::shared_ptr<SDLEmuAdapter> emulator)
{
    Emulator_ = emulator;
    display_ = true;
    initializeWindow();
}

void DebuggingWindow::HandleEvent(const SDL_Event &event)
{
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
        ImGui::ShowDemoWindow(NULL);
        drawStateInfoPane();
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
    ImGui::Begin("State Info");
    const auto &cpu = Emulator_->GetCPU();
    ImGui::Text("PC: %04X", cpu.GetPC());

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
    window_ = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
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