#pragma once

#include "engine.h"

class Game {
public:
    Game();
    ~Game();

    void Tick();
    bool Finished();
    void Quit();

private:
    GameEngine game;
    bool running = true;

    bool upKey = false;
    bool downKey = false;
    bool leftKey = false;
    bool rightKey = false;

    void SetupListeners();
    void SetupBlocks();
    void MovePlayer();
    void UpdateCamera();
};
