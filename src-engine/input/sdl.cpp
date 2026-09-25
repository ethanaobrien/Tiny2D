#include "input.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <ostream>

// Map a raw SDL keycode to one of our action keys.
// Modifier keys (Ctrl/Shift/Alt) are intentionally ignored here - they are
// read live from SDL_GetModState() in Tick() so they can never get stuck.
// Returns ActionKey::None for keys we don't track.
static ActionKey TranslateKey(std::uint32_t sym) {
    switch (sym) {
        case SDLK_UP:    return ActionKey::Up;
        case SDLK_DOWN:  return ActionKey::Down;
        case SDLK_LEFT:  return ActionKey::Left;
        case SDLK_RIGHT: return ActionKey::Right;
        case SDLK_a:     return ActionKey::A;
          // ...add more mappings here
        default:         return ActionKey::None;
     }
}

// SDL backend: no extra wiring needed - Tick() polls the queue itself.
void InputDriver::SetupBackend() {}

// Polls SDL events, handles window and mouse callbacks, and dispatches the current keys
void InputDriver::Tick() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN: {
                auto k = TranslateKey(event.key.keysym.sym);
                if (k != ActionKey::None) SetKey(k, true);
                break;
              }
            case SDL_KEYUP: {
                auto k = TranslateKey(event.key.keysym.sym);
                if (k != ActionKey::None) SetKey(k, false);
                break;
              }
            case SDL_MOUSEBUTTONDOWN:
                if (onClick) onClick(event.button.x, event.button.y);
                break;
            case SDL_QUIT:
                if (onClose) onClose();
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED && onResize)
                    onResize();
                break;
            default:
                break;
          }
      }

    std::uint32_t mods = 0;
    SDL_Keymod m = SDL_GetModState();
    if (m & KMOD_CTRL)  mods |= static_cast<std::uint32_t>(ActionKey::Ctrl);
    if (m & KMOD_SHIFT) mods |= static_cast<std::uint32_t>(ActionKey::Shift);
    if (m & KMOD_ALT)   mods |= static_cast<std::uint32_t>(ActionKey::Alt);
    Dispatch(held | mods);
}
