#include "input.h"

#ifndef __EMSCRIPTEN__
#error "Emscripten input driver was enabled on not emscripten platform!!"
#endif
#include <emscripten.h>
#include <emscripten/html5.h>

// The event-driven backend: browser key events push into the held-state, and
// Tick() (called every frame by Game::Tick) just dispatches. The held-state
// update needs a driver pointer, which is this InputDriver's own address.
static EM_BOOL key_callback(int event_type,
   const EmscriptenKeyboardEvent *key_event, void *data)
{
    auto* driver = static_cast<InputDriver*>(data);
    bool down = (event_type == EMSCRIPTEN_EVENT_KEYDOWN);

    if (strcmp("ArrowUp", key_event->key) == 0)
        driver->SetKey(ActionKey::Up, down);
    if (strcmp("ArrowDown", key_event->key) == 0)
        driver->SetKey(ActionKey::Down, down);
    if (strcmp("ArrowLeft", key_event->key) == 0)
        driver->SetKey(ActionKey::Left, down);
    if (strcmp("ArrowRight", key_event->key) == 0)
        driver->SetKey(ActionKey::Right, down);

    return EM_TRUE;
}

// Wire the browser's key events to this driver. `this` is stable (a value
// member of GameEngine), so passing it as the callback's user data is safe.
void InputDriver::SetupBackend() {
    emscripten_set_keydown_callback("!canvas", this, false, key_callback);
    emscripten_set_keyup_callback("!canvas", this, false, key_callback);
}

// Emscripten is event-driven, so Tick() just dispatches - the callbacks above
// already updated the held-state.
void InputDriver::Tick() {
    Dispatch(held);
}
