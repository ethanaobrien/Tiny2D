#pragma once
#include "colors.h"
#include "gameobject.h"
#include "engine.h"
#include <chrono>
#include <unordered_map>

#include <SDL2/SDL.h>
#ifndef DISABLE_TTF
#include <SDL2/SDL_ttf.h>
#endif

class GameEngine;
class GameObject;

struct WindowSize {
    int width;
    int height;
    
    // Stores window dimensions, defaulting to an empty size
    WindowSize(int width = 0, int height = 0) :
        width(width),
        height(height)
    {}
};

class DrawingEngine {
private:
    WindowSize window;

    SDL_Window *swindow;
    SDL_Renderer *renderer;
#ifndef DISABLE_TTF
    TTF_Font *font;
    std::unordered_map<std::string, SDL_Texture*> textCache;
#endif
    
#ifdef DEBUG_STATS
    std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdateFpsTime;
    int lastFps;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime;
    double UpdateFrameTime();
#endif

    void DrawObject(GameObject *obj);
    void ClearScreen();
    void FillRect(int x, int y, int width, int height, Color color);
    void DrawBorder(int x, int y, int width, int height, Color color);
    void DrawText(int x, int y, int ptsize, std::string text, Color color);
    void LogStartup();
    void HandleResize();

    GameEngine *engine;
public:
    void RenderFrame(std::list<GameObject> objects);
    void ResizeWindow();
    void RedrawObject(GameObject *obj);

    WindowSize GetWindowSize();

    DrawingEngine(GameEngine *engine);
    ~DrawingEngine();
};
