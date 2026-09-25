#include "game/game.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <SDL2/SDL.h>
#endif

int main() {
    static Game game;
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(
        [](void* data) { static_cast<Game*>(data)->Tick(); }, &game, 60, 1);
#else
    while (game.Running()) {
        const auto start = SDL_GetTicks();
        game.Tick();
        const auto elapsed = SDL_GetTicks() - start;
        if (elapsed < 16) SDL_Delay(16 - elapsed);
    }
#endif
    return 0;
}
