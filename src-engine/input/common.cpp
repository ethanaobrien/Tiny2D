#include "input.h"

// Adds a callback for a key or combination. Arguments are active now and active last tick.
void InputDriver::AddListener(ActionKey combo, KeyCallback callback) {
    listeners[static_cast<std::uint32_t>(combo)].push_back(std::move(callback));
}

// Sets or clears a key bit in the held state
void InputDriver::SetKey(ActionKey key, bool down) {
    //std::cout << "Set Key: " << key << ", down: " << down << std::endl;
    const std::uint32_t bit = static_cast<std::uint32_t>(key);
    if (down) held |= bit;
    else      held &= ~bit;
}

// Runs matching callbacks every tick while held and once when released
void InputDriver::Dispatch(std::uint32_t active) {
    for (auto& [combo, callbacks] : listeners) {
        if (combo == 0) continue;
        auto activeThisTick = (active & combo) == combo;
        auto activeLastTick = (prevActive & combo) == combo;
        if (activeThisTick) {
            for (auto& cb : callbacks)
                cb(true, activeLastTick);
        }

        if (activeLastTick && !activeThisTick) {
            for (auto& cb : callbacks)
                cb(false, false);
        }
    }

    prevActive = active;
}
