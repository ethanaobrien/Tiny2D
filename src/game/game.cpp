#include "game.h"
#include <iostream>

constexpr double PLAYER_GRAVITY = 0.75;
constexpr int WORLD_WIDTH = 2000;

// Move the player if the keys are being pressed
void Game::MovePlayer() {
    auto speed = 4;
    auto player = this->game.GetObjectRef("!player");
    if (player == nullptr) return;

    if (this->leftKey) {
        player->Move(-speed, 0);
    }
    if (this->rightKey) {
        player->Move(speed, 0);
    }
    // Jump only needs to be requested once per press
    if (this->upKey && player->IsGrounded()) {
        player->Jump();
    }
}

// Setup blocks the player can interact with
void Game::SetupBlocks() {
    this->game.RemoveAllObjects();
    this->game.camera.Reset();

    // Ground
    auto ground = GameObject(Color::Green(), true, false, 0);
    ground.Init(0, 640, WORLD_WIDTH, 80, "ground", false);
    this->game.AddObject(ground);

    // The red box can be pushed around
    auto box = GameObject(Color::Red(), true, true, 0);
    box.Init(400, 600, 40, 40, "box-red", true);
    box.text = "PUSH ME";
    this->game.AddObject(box);

    // Blue box that cannot be pushed
    auto crate = GameObject(Color(30, 60, 220), true, false, 0);
    crate.Init(700, 600, 40, 40, "box-blue", false);
    crate.text = "CANT PUSH";
    this->game.AddObject(crate);

    // A block to jump up onto
    auto step = GameObject(Color::Gray(), true, false, 0);
    step.Init(950, 560, 200, 20, "step", false);
    this->game.AddObject(step);

    // The player pushes entities it touches
    auto player = GameObject(Color::Yellow(), true, true, PLAYER_GRAVITY);
    player.Init(60, 580, 40, 40, "!player", true);
    player.canPushEntities = true;
    this->game.AddObject(player);
}

// Move camera when the player gets beyond a certain x value
void Game::UpdateCamera() {
    auto player = this->game.GetObjectRef("!player");
    if (player == nullptr) return;
    auto window = this->game.GetWindowSize();
    auto target = player->x - (window.width / 3);
    if (target < 0) {
        target = 0;
    }
    if (target > WORLD_WIDTH - window.width) {
        target = WORLD_WIDTH - window.width;
    }
    this->game.camera.x = target;
}

// Setup listeners (key presses / window resize / window close)
void Game::SetupListeners() {
    this->game.input.SetupBackend();
    this->game.input.AddListener(ActionKey::Up,    [this](bool held, bool repeated){ upKey     = held; });
    this->game.input.AddListener(ActionKey::Down,  [this](bool held, bool repeated){ downKey   = held; });
    this->game.input.AddListener(ActionKey::Left,  [this](bool held, bool repeated){ leftKey   = held; });
    this->game.input.AddListener(ActionKey::Right, [this](bool held, bool repeated){ rightKey  = held; });
    this->game.input.onClose  = [this]() { Quit(); };
    this->game.input.onResize = [this]() { game.ResizeWindow(); };
}

// Setup listeners and blocks
Game::Game() {
    this->running = true;

    this->SetupListeners();
    this->SetupBlocks();
}

// Deconstructor (destroy anything we created)
Game::~Game() {
}

// Quit the game
void Game::Quit() {
    this->running = false;
}

// Whether or not the game is running
bool Game::Finished() {
    return !this->running;
}

// Called every frame, to process physics/movement/camera/etc
void Game::Tick() {
    if (this->Finished()) return;
    this->game.input.Tick();
    this->MovePlayer();
    this->UpdateCamera();
    this->game.Tick();
}
