#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include "collider.h"
#include "engine.h"
#include "colors.h"
#include "direction.h"

class GameEngine;

class GameObjectUserData {
public:
    void* data = nullptr;
    std::function<void(void*)> deleter = nullptr;

    // Calls the user supplied cleanup function when the last shared data holder is destroyed
    ~GameObjectUserData() {
        if (data != nullptr && deleter != nullptr) {
            deleter(data);
        }
    }
};

class GameObject {
    int pendingX;
    int pendingY;
    double gravity;
    bool entity;

    double verticalVelocity = 0.0;
    double verticalRemainder = 0.0;
    bool grounded = false;
    bool pendingJump = false;
    bool gravityEnabled = true;

    bool canGoDown;
    bool canGoUp;
    bool canGoLeft;
    bool canGoRight;

    bool forcedY = false;
    
    GameEngine *engine;

    std::shared_ptr<GameObjectUserData> userData;
public:
    std::string id;
    int objectId;

    int x;
    int y;
    int width;
    int height;
    bool collider;
    bool canPushEntities = false;
    bool canMove;
    bool IsEntity();
    void ForceSetPosition(int x, int y);
    Color border = Color::None();
    Color color;
    std::string text;

    void Init(int x, int y, int width, int height, std::string id, bool entity);
    void Move(int diffx, int diffy);
    void Jump();
    void StopJumping();
    void ResetVerticalMotion();
    void SetVerticalVelocity(double velocity);
    void SetGravityEnabled(bool enabled);
    bool IsGrounded() const;

    int TickPendingX(ColliderRV canMove);
    int TickPendingY(ColliderRV canMove);
    void ResetPendingValues();
    void TickVerticalMotion();
    bool OutOfBounds();
    bool OutOfBounds(bool x, bool y);
    void SetGameEngine(GameEngine *engine);
    
    bool CanGoUp();
    bool CanGoDown();
    bool CanGoLeft();
    bool CanGoRight();

    int GetPendingX();
    int GetPendingY();

    Direction moveRestricted = Direction::None;
    void ForceCanGoDown(bool value);
    
    // Fired after physics, once per object and side touched by movement.
    std::function<void(GameObject&, CollisionSide)> OnCollision;

    void* GetDataRef();
    void SetData(void* data);
    void SetData(void* data, std::function<void(void*)> autofree);

    // Sets appearance and physics options and creates the shared user data holder. Call Init next.
    GameObject(Color color, bool collider, bool canMove, double gravity)
        : color(color), collider(collider), canMove(canMove), gravity(gravity) {
        this->userData = std::make_shared<GameObjectUserData>();
    }

    // Compares objects by their string IDs
    bool operator == (const GameObject& other) const {
        return this->id == other.id;
    }

    // Compares string IDs through a pointer, returning false for nullptr
    bool operator == (const GameObject* other) const {
        if (other == nullptr) return false;
        return this->id == other->id;
    }

    // Returns whether the objects have different string IDs
    bool operator != (const GameObject& other) const {
        return this->id != other.id;
    }
};
