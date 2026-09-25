#pragma once
#include <cstdint>

enum class Direction : std::uint32_t {
    None  = 0,
    Up    = 1u << 0,
    Down  = 1u << 1,
    Left  = 1u << 2,
    Right = 1u << 3,
};

// Combines direction flags so several directions can be restricted at once
inline Direction operator | (Direction a, Direction b) {
    return static_cast<Direction>(std::uint32_t(a) | std::uint32_t(b));
}
// Adds direction flags to an existing restriction
inline Direction& operator |= (Direction& a, Direction b) {
    a = static_cast<Direction>(std::uint32_t(a) | std::uint32_t(b));
    return a;
}
// Gets the flags shared by both values, used to check a direction
inline Direction operator & (Direction a, Direction b) {
    return static_cast<Direction>(std::uint32_t(a) & std::uint32_t(b));
}
