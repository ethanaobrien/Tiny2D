#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <ostream>

// ---------------------------------------------------------------------------
// ActionKey - our own bit flags.
//
// Raw SDL keycodes are NOT bit flags, so you cannot OR them together:
//     SDLK_a | SDLK_b   ==  SDLK_c     (silently wrong)
// Each ActionKey is a single distinct bit, so combos compose cleanly:
//     ActionKey::Ctrl | ActionKey::A
// ---------------------------------------------------------------------------
enum class ActionKey : std::uint32_t {
    None  = 0,
    Up    = 1u << 0,
    Down  = 1u << 1,
    Left  = 1u << 2,
    Right = 1u << 3,
    Ctrl  = 1u << 4,
    Shift = 1u << 5,
    Alt   = 1u << 6,
    A     = 1u << 7,
};

// Gets a readable name for the listed keys, or Unknown for other values and combinations
inline const char* toString(ActionKey key) {
    switch (key) {
        case ActionKey::None: return "None";
        case ActionKey::Up:   return "Up";
        case ActionKey::Down: return "Down";
        case ActionKey::Left: return "Left";
        case ActionKey::Right:return "Right";
        case ActionKey::A:    return "A";
        default:              return "Unknown";
    }
}

// Allows an action key name to be printed to an output stream
inline std::ostream& operator<<(std::ostream& os, ActionKey key) {
    return os << toString(key);
}

// Combine / compare flags with the bitwise operators:
// Combines keys into a single key combination
inline ActionKey operator|(ActionKey a, ActionKey b) {
    return static_cast<ActionKey>(std::uint32_t(a) | std::uint32_t(b));
}
// Gets the key bits shared by both values
inline ActionKey operator&(ActionKey a, ActionKey b) {
    return static_cast<ActionKey>(std::uint32_t(a) & std::uint32_t(b));
}

using KeyCallback   = std::function<void(bool, bool)>;
using MouseCallback = std::function<void(int, int)>;
using VoidCallback  = std::function<void()>;

// InputDriver is a plain value type owned by GameEngine, so other systems
// register against it:  game.input.AddListener(ActionKey::Up, ...).
// It knows nothing about Game - the callbacks capture whatever they need.
class InputDriver {
public:
    // Creates an input driver with no held keys or registered listeners
    InputDriver() = default;

       // Register a callback that fires every frame while `combo` is held.
    void AddListener(ActionKey combo, KeyCallback callback);

       // Poll events (SDL) / dispatch (emscripten) and fire matching callbacks.
       // Call once per frame.
    void Tick();

       // Drive the held-state directly. Used by the event-driven emscripten
       // backend; the SDL backend updates held-state inside Tick().
    void SetKey(ActionKey key, bool down);

       // One-time backend wiring. No-op for SDL; registers emscripten key
       // events for the browser build. Call once from Game's constructor.
    void SetupBackend();

      // One-shot events. Assign a callback or leave it null (skipped in Tick).
    MouseCallback onClick;     // SDL_MOUSEBUTTONDOWN -> (x, y)
    VoidCallback  onClose;     // SDL_QUIT
    VoidCallback  onResize;    // SDL_WINDOWEVENT (window resized)

private:
    std::unordered_map<std::uint32_t, std::vector<KeyCallback>> listeners;
    std::uint32_t held = 0;            // bitmask of currently-held non-modifier keys
    std::uint32_t prevActive = 0;      // last frame's active mask, for edge detection

    void Dispatch(std::uint32_t active); // active = held, some platforms need to remove some modifiers here so this is easier
};
