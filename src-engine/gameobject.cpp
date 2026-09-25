#include "gameobject.h"
#include "colors.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include "direction.h"

namespace {
constexpr double JUMP_IMPULSE = 22.0;       // pixels per physics step
constexpr double JUMP_RELEASE_SPEED = 10.0; // remaining upward speed on release
constexpr double MAX_FALL_SPEED = 16.0;
}

// Sets the position, size, and ID and resets motion. Call this before adding the object to the engine.
void GameObject::Init(int x, int y, int width, int height, std::string id, bool entity) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->id = id;
    this->pendingX = 0;
    this->pendingY = 0;
    this->verticalVelocity = 0.0;
    this->verticalRemainder = 0.0;
    this->grounded = false;
    this->gravityEnabled = true;
    this->engine = nullptr;
    this->entity = entity;
    this->pendingJump = false;
    this->text = "";
}

// Returns whether this object was marked as an entity during Init
bool GameObject::IsEntity() {
    return this->entity;
}

// Adds movement to be applied during physics, so collisions can limit how far it goes
void GameObject::Move(int diffx, int diffy) {
    this->pendingX += diffx;
    this->pendingY += diffy;
}

// Requests a jump for the next physics step. It only happens if the object is grounded.
void GameObject::Jump() {
    this->pendingJump = true;
}

// Limits the remaining upward speed when the jump key is released
void GameObject::StopJumping() {
    // Releasing jump shortens the ascent without cancelling it outright.
    this->verticalVelocity = std::max(this->verticalVelocity, -JUMP_RELEASE_SPEED);
}

// Clears vertical speed, leftover fractional movement, and any pending jump
void GameObject::ResetVerticalMotion() {
    this->verticalVelocity = 0.0;
    this->verticalRemainder = 0.0;
    this->pendingJump = false;
}

// Sets vertical speed in pixels per physics step. Negative values move up.
void GameObject::SetVerticalVelocity(double velocity) {
    this->verticalVelocity = velocity;
    this->verticalRemainder = 0.0;
}

// Enables or disables gravity without clearing the current vertical speed
void GameObject::SetGravityEnabled(bool enabled) {
    this->gravityEnabled = enabled;
}

// Returns whether the last movement check placed the object on a surface
bool GameObject::IsGrounded() const {
    return this->grounded;
}

// Moves directly to this position without checking collisions and resets vertical motion
void GameObject::ForceSetPosition(int x, int y) {
    this->x = x;
    this->y = y;
    this->ResetVerticalMotion();
    this->grounded = false;
}

// Gets the user data pointer shared by copies of this object
void* GameObject::GetDataRef() {
    return this->userData->data;
}
// Stores a user data pointer. Any previously assigned cleanup callback is kept.
void GameObject::SetData(void* data) {
    this->userData->data = data;
}
// Stores user data and its cleanup callback. Replacing the pointer does not free the old data.
void GameObject::SetData(void* data, std::function<void(void*)> deleter) {
    this->userData->data = data;
    this->userData->deleter = deleter;
}

// Clears queued movement on both axes
void GameObject::ResetPendingValues() {
    this->pendingX = 0;
    this->pendingY = 0;
}

// Gets the horizontal movement queued for the next physics step
int GameObject::GetPendingX() {
    return this->pendingX;
}

// Gets the vertical movement queued for the next physics step
int GameObject::GetPendingY() {
    return this->pendingY;
}

// Overrides vertical movement flags for the next Y update when another object pushes this one
void GameObject::ForceCanGoDown(bool value) {
    this->forcedY = true;
    this->canGoDown = value;
    this->canGoUp = !value;
    this->grounded = !value;
}

// Applies queued horizontal movement up to the collision limit and clears the queue.
// The return value keeps the requested sign except when left movement is clipped to a positive gap.
int GameObject::TickPendingX(ColliderRV canMove) {
    if (!this->canMove) {
        this->pendingX = 0;
        return 0;
    }
    auto pending = this->pendingX;
    if (pending > 0) {
        if (canMove.allowMoveXR >= pending || canMove.allowMoveXR == -1) {
            this->x += pending;
        } else {
            pending = canMove.allowMoveXR;
            this->x += canMove.allowMoveXR;
        }
    } else if (pending < 0) {
        if (canMove.allowMoveXL >= -pending || canMove.allowMoveXL == -1) {
            this->x += pending;
        } else {
            pending = canMove.allowMoveXL;
            this->x -= canMove.allowMoveXL;
        }
    }
    this->canGoLeft = (canMove.allowMoveXL != 0);
    this->canGoRight = (canMove.allowMoveXR != 0);
    this->pendingX = 0;
    return pending;
}

// Applies queued vertical movement and updates grounded state. Returns the signed distance moved.
int GameObject::TickPendingY(ColliderRV canMove) {
    if (!this->canMove) {
        this->pendingY = 0;
        return 0;
    }
    auto pending = this->pendingY;
    const int startY = this->y;
    if (pending > 0) {
        if (canMove.allowMoveYD >= pending || canMove.allowMoveYD == -1) {
            this->y += pending;
        } else {
            this->y += canMove.allowMoveYD;
        }
    } else if (pending < 0) {
        if (canMove.allowMoveYU >= -pending || canMove.allowMoveYU == -1) {
            this->y += pending;
        } else {
            this->y -= canMove.allowMoveYU;
        }
    }
    if (!this->forcedY) {
        this->canGoDown = (canMove.allowMoveYD != 0);
        this->canGoUp = (canMove.allowMoveYU != 0);
    } else {
        this->forcedY = false;
    }
    const bool hitFloor = pending >= 0 && canMove.allowMoveYD == 0;
    const bool landed = pending > 0 && canMove.allowMoveYD > 0 && canMove.allowMoveYD <= pending;
    const bool hitCeiling = pending < 0 && canMove.allowMoveYU >= 0 && canMove.allowMoveYU <= -pending;
    this->grounded = hitFloor || landed;
    if (this->grounded && this->verticalVelocity > 0.0) {
        this->verticalVelocity = 0.0;
        this->verticalRemainder = 0.0;
    }
    if (hitCeiling && this->verticalVelocity < 0.0) {
        this->verticalVelocity = 0.0;
        this->verticalRemainder = 0.0;
    }
    this->pendingY = 0;
    return this->y - startY;
}

// Applies a pending jump and gravity, then queues whole pixels of vertical movement.
// Keeps the fractional part for the next step so small movements are not lost.
void GameObject::TickVerticalMotion() {
    if (this->pendingJump && this->grounded) {
        this->verticalVelocity = -JUMP_IMPULSE;
        this->verticalRemainder = 0.0;
        this->grounded = false;
    }
    this->pendingJump = false;

    if (this->gravityEnabled && this->gravity > 0.0) {
        this->verticalVelocity = std::min(this->verticalVelocity + this->gravity, MAX_FALL_SPEED);
    }
    if ((this->moveRestricted & Direction::Down) == Direction::Down && this->verticalVelocity > 0.0) {
        this->verticalVelocity = 0.0;
        this->verticalRemainder = 0.0;
    }

    const double movement = this->verticalVelocity + this->verticalRemainder;
    const int wholePixels = static_cast<int>(std::trunc(movement));
    this->verticalRemainder = movement - wholePixels;
    this->pendingY += wholePixels;
}

// Checks whether the object is outside the camera view on either axis
bool GameObject::OutOfBounds() {
    return this->OutOfBounds(true, true);
}

// Checks the selected axes against the camera view. Objects without an engine are kept visible.
bool GameObject::OutOfBounds(bool checkX, bool checkY) {
    if (this->engine == nullptr) {
        std::cout << "Engine is null!! This is not ok! Object will always render." << std::endl;
        return false;
    }
    auto x = this->engine->camera.GetX(this->x);
    auto y = this->engine->camera.GetY(this->y);
    
    auto rightX = x + this->width;
    auto leftX = x;
    auto topY = y;
    auto bottomY = y + this->height;
    auto window = this->engine->GetWindowSize();
    if (leftX > window.width || rightX < 0) {
        if (checkX) {
            return true;
        }
    }
    if (topY > window.height || bottomY < 0) {
        if (checkY) {
            return true;
        }
    }
    return false;
}

// Stores the engine pointer used for camera and window bounds checks
void GameObject::SetGameEngine(GameEngine *engine) {
    this->engine = engine;
}

// Gets the upward movement flag from the last movement update
bool GameObject::CanGoUp() {
    return canGoUp;
}

// Gets the downward movement flag from the last movement update
bool GameObject::CanGoDown() {
    return canGoDown;
}

// Gets the left movement flag from the last horizontal update
bool GameObject::CanGoLeft() {
    return canGoLeft;
}

// Gets the right movement flag from the last horizontal update
bool GameObject::CanGoRight() {
    return canGoRight;
}
