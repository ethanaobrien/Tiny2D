#include <SDL2/SDL.h>
#include <chrono>
#include <cmath>
#include <iostream>
#include "drawing.h"
#include "gameobject.h"
#include "colors.h"

#ifndef DISABLE_TTF
#include <SDL2/SDL_ttf.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __EMSCRIPTEN__
const auto FONTPATH = "/vendor/SuperShiny-0v0rG.ttf";
#else
const auto FONTPATH = "vendor/SuperShiny-0v0rG.ttf";
#endif
const auto FONTSIZE = 54;

// Sets up SDL, the window, and the renderer, and loads the font when text is enabled
DrawingEngine::DrawingEngine(GameEngine *engine) {
    this->engine = engine;
    this->window = WindowSize(1280, 720);
    //SDL_SetHint(SDL_HINT_EMSCRIPTEN_CANVAS_SELECTOR, "!canvas"); // sdl 3...
    
    SDL_Init(SDL_INIT_VIDEO);
    this->swindow = SDL_CreateWindow("Game", 50, 50, this->window.width, this->window.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    this->renderer = SDL_CreateRenderer(this->swindow, -1, SDL_RENDERER_ACCELERATED);
    this->LogStartup();

#ifndef DISABLE_TTF
    if (TTF_Init() < 0) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
    }

    this->font = TTF_OpenFont(FONTPATH, FONTSIZE);
    if (this->font == nullptr) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    }
#endif

#ifdef DEBUG_STATS
    this->lastFrameTime = std::chrono::high_resolution_clock::now();
#endif
    this->HandleResize();
}

// Frees the font and cached text textures, then closes the renderer and window
DrawingEngine::~DrawingEngine() {
#ifndef DISABLE_TTF
    if (this->font != nullptr) {
        TTF_CloseFont(this->font);
        this->font = nullptr;
    }
    for (const auto &entry : this->textCache) {
        SDL_DestroyTexture(entry.second);
    }
    this->textCache.clear();
    TTF_Quit();
#endif
    if (this->renderer != nullptr) {
        SDL_DestroyRenderer(this->renderer);
        this->renderer = nullptr;
    }
    if (this->swindow != nullptr) {
        SDL_DestroyWindow(this->swindow);
        this->swindow = nullptr;
    }
    SDL_Quit();
}

// Prints whether SDL is using an accelerated renderer
void DrawingEngine::LogStartup() {
    SDL_RendererInfo info = {0};
    if (SDL_GetRendererInfo(this->renderer, &info) != 0) {
        return;
    }
    if (info.flags & SDL_RENDERER_ACCELERATED) {
        std::cout << "Acceleration enabled!" << std::endl;
    } else {
        std::cout << "Acceleration disabled!" << std::endl;
    }
}

// Placeholder for any extra work needed after the window size changes
void DrawingEngine::HandleResize() {
    // Nothing needed here right now
}

// Reads the window or canvas size and resets the rendering viewport
void DrawingEngine::ResizeWindow() {
#ifdef __EMSCRIPTEN__
    auto width = EM_ASM_INT({ return Module.canvas.getBoundingClientRect().width; });
    auto height = EM_ASM_INT({ return Module.canvas.getBoundingClientRect().height; });
#elif defined(__SWITCH__)
    auto width = 1280;
    auto height = 720;
#else
    auto width = SDL_GetWindowSurface(this->swindow)->w;
    auto height = SDL_GetWindowSurface(this->swindow)->h;
#endif
    this->window = WindowSize(width, height);
    std::cout << "Window resize: (" << width << ", " << height << ")" << std::endl;

#ifndef __SWITCH__
    SDL_SetWindowSize(this->swindow, width, height);
#endif

    SDL_RenderSetViewport(this->renderer, nullptr);

    this->HandleResize();
}

// Gets the stored window dimensions in pixels
WindowSize DrawingEngine::GetWindowSize() {
    return this->window;
}

// Draws a visible object relative to the camera, including its border and text
void DrawingEngine::DrawObject(GameObject *obj) {
    if (obj->OutOfBounds()) {
        return;
    }
    auto x = this->engine->camera.GetX(obj->x);
    auto y = this->engine->camera.GetY(obj->y);
    this->FillRect(x, y, obj->width, obj->height, obj->color);
    if (obj->border != Color::None()) {
        this->DrawBorder(x, y, obj->width, obj->height, obj->border);
    }
    if (obj->text != "") {
        auto textX = x + (obj->width / 2) - (FONTSIZE / 4);
        auto textY = y + (obj->height / 2) - (FONTSIZE / 4);
        this->DrawText(textX, textY, FONTSIZE, obj->text, Color::Black());
    }
}

// Draws this object into the current frame without clearing or presenting it
void DrawingEngine::RedrawObject(GameObject *object) {
    this->DrawObject(object);
}

// Clears the frame with the background color
void DrawingEngine::ClearScreen() {
    SDL_SetRenderDrawColor(this->renderer, 238, 238, 238, 255);
    SDL_RenderClear(this->renderer);
}

// Draws text using a cached texture, creating one if this text, size, and color are new
void DrawingEngine::DrawText(int x, int y, int ptsize, std::string text, Color color) {
#ifndef DISABLE_TTF
    if (this->font == nullptr) {
        return;
    }
    TTF_SetFontSize(this->font, ptsize);

    const std::string key = std::to_string(ptsize) + "\1" + text + "\1" + color.ToString();
    const auto it = this->textCache.find(key);
    if (it != this->textCache.end()) {
        SDL_Rect textRect = { x, y, 0, 0 };
        SDL_QueryTexture(it->second, nullptr, nullptr, &textRect.w, &textRect.h);
        SDL_RenderCopy(this->renderer, it->second, NULL, &textRect);
        return;
    }

    SDL_Color textColor = { color.r, color.g, color.b, color.a };
    auto surface = TTF_RenderText_Solid(this->font, text.c_str(), textColor);
    if (surface == nullptr) {
        std::cerr << "Error creating text surface: " << TTF_GetError() << std::endl;
        return;
    }

    auto textTexture = SDL_CreateTextureFromSurface(this->renderer, surface);
    SDL_Rect textRect = { x, y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    if (textTexture == nullptr) {
        std::cerr << "Error creating text texture: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_RenderCopy(this->renderer, textTexture, NULL, &textRect);
    this->textCache[key] = textTexture;
#endif
}

// Draws a filled rectangle at the supplied screen coordinates
void DrawingEngine::FillRect(int x, int y, int width, int height, Color color) {
    SDL_SetRenderDrawColor(this->renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderFillRect(this->renderer, &rect);
}

// Draws a three pixel border around the rectangle
void DrawingEngine::DrawBorder(int x, int y, int width, int height, Color color) {
    auto borderWidth = 3;
    SDL_SetRenderDrawColor(this->renderer, color.r, color.g, color.b, color.a);
    {
        SDL_Rect rect = {x, y, width + borderWidth, borderWidth};
        SDL_RenderFillRect(this->renderer, &rect);
    }

    {
        SDL_Rect rect = {x, y, borderWidth, height + borderWidth};
        SDL_RenderFillRect(this->renderer, &rect);
    }

    {
        SDL_Rect rect = {x + width, y, borderWidth, height + borderWidth};
        SDL_RenderFillRect(this->renderer, &rect);
    }

    {
        SDL_Rect rect = {x, y + height, width + borderWidth, borderWidth};
        SDL_RenderFillRect(this->renderer, &rect);
    }
}

// Clears the screen, draws the objects in list order, and presents the finished frame
void DrawingEngine::RenderFrame(std::list<GameObject> objects) {
    this->ClearScreen();
    for (auto &i: objects) {
        this->DrawObject(&i);
    }

#ifdef DEBUG_STATS
    this->DrawText(10, 10, FONTSIZE, "camera.x: " + std::to_string(this->engine->camera.x), Color::Black());
    this->DrawText(10, 10 + FONTSIZE, FONTSIZE, "camera.y: " + std::to_string(this->engine->camera.y), Color::Black());
    this->DrawText(10, 10 + FONTSIZE + FONTSIZE, FONTSIZE, "GameObjects: " + std::to_string(this->engine->GetGameObjectCount()), Color::Black());
    this->DrawText(10, 10 + FONTSIZE + FONTSIZE + FONTSIZE, FONTSIZE, "FPS: " + std::to_string(static_cast<int>(std::round(this->UpdateFrameTime()))), Color::Black());
#endif

    SDL_RenderPresent(this->renderer);
}

#ifdef DEBUG_STATS
// Samples the most recent frame time once a second and returns the stored FPS estimate
double DrawingEngine::UpdateFrameTime() {
    auto current = std::chrono::high_resolution_clock::now();

    double durationSinceUpdate = std::chrono::duration_cast<std::chrono::duration<double>>(current - this->lastUpdateFpsTime).count();

    if (durationSinceUpdate >= 1.0) {
        double duration = std::chrono::duration_cast<std::chrono::duration<double>>(current - this->lastFrameTime).count();
        this->lastFps = 1.0 / duration;
        this->lastUpdateFpsTime = current;
    }

    this->lastFrameTime = current;

    return lastFps;
}
#endif
