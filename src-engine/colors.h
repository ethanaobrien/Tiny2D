#pragma once
#include <string>
#include <cstdint>

class Color {
public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    // Creates an opaque white color
    Color() : r(255), g(255), b(255), a(255) {}
    // Creates a color from red, green, blue, and optional alpha values
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    // Checks for the transparent white value used to mean no color
    bool IsNone() const { return r == 255 && g == 255 && b == 255 && a == 0; }

    // Compares all four color channels
    bool operator == (const Color &o) const {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }

    // Returns whether any of the color channels differ
    bool operator != (const Color &o) const {
        return !(*this == o);
    }

    // "r,g,b" (or "r,g,b,a" when alpha < 255).
    std::string ToString() const {
        std::string s = std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b);
        if (a != 255) {
            s += "," + std::to_string(a);
        }
        return s;
    }

    // Gets the transparent value used to disable borders
    static Color None()   { return Color(255, 255, 255, 0); }
    // Gets the preset red color
    static Color Red()    { return Color(255, 0, 0); }
    // Gets the preset black color
    static Color Black()  { return Color(0, 0, 0); }
    // Gets the preset brown color
    static Color Brown()  { return Color(165, 42, 42); }
    // Gets the preset green color
    static Color Green()  { return Color(0, 128, 0); }
    // Gets the preset yellow color
    static Color Yellow() { return Color(225, 225, 0); }
    // Gets the preset gray color
    static Color Gray()   { return Color(128, 128, 128); }
    // Gets the preset purple color
    static Color Purple() { return Color(160, 32, 240); }
    // Gets the preset orange color
    static Color Orange() { return Color(255, 165, 0); }
};
