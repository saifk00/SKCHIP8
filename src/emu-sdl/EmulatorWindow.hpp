#ifndef _EMULATOR_WINDOW_H
#define _EMULATOR_WINDOW_H

#include <SDL.h>

#include "SDLEmuAdapter.h"

#include <memory>

static constexpr int SCR_WIDTH = 640;
static constexpr int SCR_HEIGHT = 320;

class EmulatorWindow
{
public:
    EmulatorWindow(std::shared_ptr<SDLEmuAdapter> emulator);
    void Update();
    Uint32 GetWindowID() const { return SDL_GetWindowID(window_); }
    void HandleEvent(const SDL_Event &event);

protected:
    void initializeWindow();
    void destroyWindow();

private:
    std::shared_ptr<SDLEmuAdapter> emulator_;
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    bool display_;
};

void EmulatorWindow::HandleEvent(const SDL_Event &event)
{
    if (!display_)
        return;

    // handle events
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_CLOSE)
    {
        destroyWindow();
    }
}

void EmulatorWindow::Update()
{
    if (!display_)
        return;

    uint64_t start = SDL_GetPerformanceCounter();

    emulator_->UpdateKeyState();

    emulator_->Update();

    auto frame = emulator_->GetFrameBuffer();

    SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer_);
    SDL_SetRenderDrawColor(renderer_, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPoints(renderer_, frame.data(), frame.size());
    SDL_RenderPresent(renderer_);

    uint64_t end = SDL_GetPerformanceCounter();
    float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency();

    SDL_Delay(16.666f - elapsed * 1000.0f);

    uint64_t end2 = SDL_GetPerformanceCounter();
    float elapsed2 = (end2 - end) / (float)SDL_GetPerformanceFrequency();
    emulator_->SetFPS(1.0 / elapsed2);
}

EmulatorWindow::EmulatorWindow(std::shared_ptr<SDLEmuAdapter> emulator)
{
    emulator_ = emulator;
    display_ = true;

    initializeWindow();
}

void EmulatorWindow::initializeWindow()
{
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (SDL_CreateWindowAndRenderer(SCR_WIDTH, SCR_HEIGHT, window_flags, &window_, &renderer_))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
    }
    SDL_RenderSetLogicalSize(renderer_, SKChip8::SCR_WIDTH, SKChip8::SCR_HEIGHT);
}

void EmulatorWindow::destroyWindow()
{
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    display_ = false;
}

#endif