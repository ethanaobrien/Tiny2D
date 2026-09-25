#include "game.h"

#include <algorithm>
#include <string>

namespace {
constexpr int kPlayerSpeed = 5;
constexpr int kTotalSquares = 5;
constexpr int kWorldWidth = 2600;
const Color sky(183, 222, 243);
const Color grass(92, 176, 109);
const Color idlePad(157, 97, 194);
const Color earth(89, 92, 111);
const Color platform(79, 115, 151);
const Color gold(255, 202, 66);
const Color ink(37, 53, 77);
}

Game::Game() {
    engine.input.AddListener(ActionKey::Left, [this](bool down, bool) {
        if (down && !won) engine.GetObjectRef("player")->Move(-kPlayerSpeed, 0);
    });
    engine.input.AddListener(ActionKey::Right, [this](bool down, bool) {
        if (down && !won) engine.GetObjectRef("player")->Move(kPlayerSpeed, 0);
    });
    engine.input.AddListener(ActionKey::Up, [this](bool down, bool wasDown) {
        if (down && !wasDown && !won) engine.GetObjectRef("player")->Jump();
    });
    engine.input.AddListener(ActionKey::Down, [this](bool down, bool wasDown) {
        if (down && !wasDown) Reset();
    });
    engine.input.onClose = [this] { running = false; };
    engine.input.onResize = [this] { engine.ResizeWindow(); };
    engine.input.SetupBackend();
    Reset();
}

void Game::AddBlock(const char* id, int x, int y, int width, int height,
                    Color color, bool solid) {
    GameObject block(color, solid, false, 0);
    block.Init(x, y, width, height, id, false);
    engine.AddObject(block);
}

void Game::AddLabel(const char* id, int x, int y, const char* text) {
    GameObject label(Color::None(), false, false, 0);
    // The renderer centers text around an object's position. A 1px object
    // lets labels sit at the specified screen coordinates.
    label.Init(x + 13, y + 13, 1, 1, id, false);
    label.text = text;
    engine.AddObject(label);
}

void Game::UpdateHud() {
    auto* hud = engine.GetObjectRef("hud");
    hud->text = "SQUARES " + std::to_string(collected) + "/5";
}

void Game::UpdateCamera() {
    auto* player = engine.GetObjectRef("player");
    const int screenWidth = engine.GetWindowSize().width;
    const int maxCameraX = std::max(0, kWorldWidth - screenWidth);
    const int playerCenter = player->x + player->GetPendingX() + player->width / 2;
    engine.camera.x = std::clamp(playerCenter - screenWidth / 2, 0, maxCameraX);

    // These are ordinary engine objects, so move them into camera space to
    // keep the interface fixed while the level scrolls behind it.
    auto* hudPanel = engine.GetObjectRef("hud_panel");
    hudPanel->x = engine.camera.x;
    hudPanel->width = screenWidth;
    auto* hudRule = engine.GetObjectRef("hud_rule");
    hudRule->x = engine.camera.x;
    hudRule->width = screenWidth;
    engine.GetObjectRef("hud")->x = engine.camera.x + 20 + 13;
    engine.GetObjectRef("controls")->x = engine.camera.x + 550 + 13;
    engine.GetObjectRef("push_tip")->x = engine.camera.x + 20 + 13;
    engine.GetObjectRef("restart")->x = engine.camera.x + 800 + 13;
    if (won) {
        engine.GetObjectRef("win_panel")->x = engine.camera.x + 345;
        engine.GetObjectRef("win")->x = engine.camera.x + 420 + 13;
        engine.GetObjectRef("win_hint")->x = engine.camera.x + 375 + 13;
    }
}

void Game::Reset() {
    engine.RemoveAllObjects();
    engine.camera.Reset();
    collected = 0;
    won = false;
    padActivated = false;

    AddBlock("sky", 0, 0, kWorldWidth, 720, sky);
    AddBlock("sun", 1080, 135, 82, 82, gold);
    AddBlock("cloud1", 110, 220, 130, 18, Color(239, 248, 250));
    AddBlock("cloud2", 570, 170, 160, 18, Color(239, 248, 250));
    AddBlock("cloud3", 930, 250, 110, 18, Color(239, 248, 250));
    AddBlock("cloud4", 1420, 180, 150, 18, Color(239, 248, 250));
    AddBlock("cloud5", 1900, 235, 130, 18, Color(239, 248, 250));
    AddBlock("cloud6", 2350, 175, 145, 18, Color(239, 248, 250));

    AddBlock("floor", 0, 640, kWorldWidth, 80, earth, true);
    AddBlock("grass", 0, 640, kWorldWidth, 12, grass);
    AddBlock("platform1", 280, 530, 160, 24, platform, true);
    AddBlock("platform2", 540, 420, 160, 24, platform, true);
    AddBlock("platform3", 800, 310, 160, 24, platform, true);
    AddBlock("platform4", 1330, 530, 160, 24, platform, true);
    AddBlock("platform5", 1640, 420, 160, 24, platform, true);
    AddBlock("platform6", 1900, 310, 160, 24, platform, true);
    AddBlock("right_wall", 2570, 0, 30, 640, platform, true);
    AddLabel("push_sign", 1040, 255, "PUSH BOX TO PAD");
    AddBlock("pad", 1470, 628, 100, 12, idlePad);
    engine.GetObjectRef("pad")->border = ink;
    AddLabel("pad_status", 1395, 190, "PAD OFF");

    const int squareX[] = {170, 345, 605, 1400, 2110};
    const int squareY[] = {601, 491, 381, 491, 601};
    for (int i = 0; i < kTotalSquares; ++i) {
        GameObject square(gold, false, false, 0);
        square.Init(squareX[i], squareY[i], 25, 25,
                    "square" + std::to_string(i), false);
        square.border = Color(206, 131, 31);
        engine.AddObject(square);
    }

    AddBlock("exit", 2450, 567, 54, 73, Color(69, 172, 145));
    AddBlock("exit_light", 2462, 581, 30, 43, Color(197, 242, 184));

    GameObject player(Color(74, 88, 201), true, true, 1.3);
    player.Init(80, 596, 32, 44, "player", true);
    player.border = ink;
    player.canPushEntities = true;
    engine.AddObject(player);

    GameObject crate(Color(204, 126, 70), true, true, 3);
    crate.Init(1160, 584, 56, 56, "crate", true);
    crate.border = ink;
    crate.text = "P";
    engine.AddObject(crate);

    AddBlock("hud_panel", 0, 0, 1280, 112, Color(243, 247, 239));
    AddBlock("hud_rule", 0, 108, 1280, 4, ink);
    AddLabel("hud", 20, 10, "SQUARES 0/5");
    AddLabel("controls", 550, 10, "ARROWS: MOVE / JUMP");
    AddLabel("push_tip", 20, 63, "WALK INTO BOX TO PUSH");
    AddLabel("restart", 800, 63, "DOWN: RESTART");
}

bool Game::Overlaps(const GameObject& a, const GameObject& b) {
    return a.x < b.x + b.width && a.x + a.width > b.x &&
           a.y < b.y + b.height && a.y + a.height > b.y;
}

void Game::Tick() {
    engine.input.Tick();
    UpdateCamera();
    engine.Tick();
    if (won) return;

    auto* player = engine.GetObjectRef("player");
    for (int i = 0; i < kTotalSquares; ++i) {
        const auto id = "square" + std::to_string(i);
        auto* square = engine.GetObjectRef(id);
        if (square && Overlaps(*player, *square)) {
            engine.RemoveObject(id);
            ++collected;
            UpdateHud();
        }
    }

    auto* pad = engine.GetObjectRef("pad");
    const bool boxOnPad = Overlaps(*engine.GetObjectRef("crate"), *pad);
    if (boxOnPad != padActivated) {
        padActivated = boxOnPad;
        pad->color = padActivated ? grass : idlePad;
        engine.GetObjectRef("pad_status")->text = padActivated ? "PAD ON!" : "PAD OFF";
    }

    if (collected == kTotalSquares &&
        Overlaps(*player, *engine.GetObjectRef("exit"))) {
        won = true;
        AddBlock("win_panel", engine.camera.x + 345, 235, 590, 155,
                 Color(243, 247, 239));
        AddLabel("win", engine.camera.x + 420, 260, "YOU WIN!");
        AddLabel("win_hint", engine.camera.x + 375, 325, "DOWN TO PLAY AGAIN");
    }
}
