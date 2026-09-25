#pragma once

#include "engine.h"

class Game {
public:
    Game();
    void Tick();
    bool Running() const { return running; }

private:
    GameEngine engine;
    bool running = true;
    bool won = false;
    bool padActivated = false;
    int collected = 0;

    void Reset();
    void AddBlock(const char* id, int x, int y, int width, int height,
                  Color color, bool solid = false);
    void AddLabel(const char* id, int x, int y, const char* text);
    void UpdateCamera();
    void UpdateHud();
    static bool Overlaps(const GameObject& a, const GameObject& b);
};
