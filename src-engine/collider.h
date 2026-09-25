#pragma once

// The side of an object that met another collider.
enum class CollisionSide { Up, Left, Down, Right };

// Movement limits in pixels for each direction. -1 means unlimited and 0 means blocked.
struct ColliderRV {
    int allowMoveXL;
    int allowMoveXR;
    int allowMoveYU;
    int allowMoveYD;
};

struct MovementInfo {
    bool canGoUp;
    bool canGoDown;
    bool canGoLeft;
    bool canGoRight;
    
    // Converts movement distances into simple flags for each direction
    MovementInfo(ColliderRV collider) {
        this->canGoUp = (collider.allowMoveYU != 0);
        this->canGoDown = (collider.allowMoveYD != 0);
        this->canGoLeft = (collider.allowMoveXL != 0);
        this->canGoRight = (collider.allowMoveXR != 0);
    }
};
