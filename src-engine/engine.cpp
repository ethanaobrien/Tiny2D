#include <iostream>
#include <algorithm>
#include <utility>
#include <cmath>
#include <array>
#include "engine.h"
#include "gameobject.h"

// TODO - compare fix comparison with values < 0 and > 0

// Creates the drawing engine and sets the camera back to the origin
GameEngine::GameEngine() {
    this->drawing = new DrawingEngine(this);
    this->ResizeWindow();
    this->camera.Reset();
}

// Frees the drawing engine, which also closes the SDL window
GameEngine::~GameEngine() {
    delete this->drawing;
}

// Updates the renderer to match the current window size
void GameEngine::ResizeWindow() {
    this->drawing->ResizeWindow();
}

// Gets the grid bucket from the distance to x = 0. Bucket 0 is reserved for wide objects.
int GameEngine::GetGridIndex(int x) {
    auto X = std::abs(x);
    return ((X - (X % this->gridBlockWidth)) / this->gridBlockWidth) + 1;
}

// Wide objects go in bucket 0 so collision checks always include them
int GameEngine::GetGridIndex(GameObject* obj) {
    if (obj->width > this->gridBlockWidth) {
        return 0; // No easy way to calculate this object... Just always check it!
    }
    return this->GetGridIndex(obj->x);
}

// Rebuilds the collision grid from the objects that can collide or push other objects
void GameEngine::UpdateGrid() {
    for (auto &i: this->grid) {
        i.clear();
    }
    for (auto &obj: this->objects) {
        if ((!obj.collider && !obj.canPushEntities) || (this->entityCollisions == false && obj.IsEntity())) {
            continue;
        }
        auto index = this->GetGridIndex(&obj);
        while (this->grid.size() <= index) {
            this->grid.push_back(std::vector<GameObject*>());
        }
        this->grid[index].push_back(&obj);
    }
    this->upToDateGrid = true;
}

// Sets whether entities are included the next time the collision grid is rebuilt
void GameEngine::SetEntityCollisions(bool check) {
    this->entityCollisions = check;
}

// Checks how far the object can move on each requested axis and queues collision events.
// A distance of -1 means no limit was found, and 0 means movement is blocked.
ColliderRV GameEngine::IsColliding(GameObject *object, bool checkX, bool checkY, int requested) {
    if (!object->collider) {
        ColliderRV rv;
        rv.allowMoveXL = 9999;
        rv.allowMoveXR = 9999;
        rv.allowMoveYU = 9999;
        rv.allowMoveYD = 9999;
        return rv;
    }
    if (!object->canMove) {
        ColliderRV rv;
        rv.allowMoveXL = 0;
        rv.allowMoveXR = 0;
        rv.allowMoveYU = 0;
        rv.allowMoveYD = 0;
        return rv;
    }
    auto allowMoveXL = -1, allowMoveXR = -1, allowMoveYU = -1, allowMoveYD = -1;
    int contactGap = -1;
    std::vector<int> contactIds;
    const auto contactSide = checkX
        ? (requested > 0 ? CollisionSide::Right : CollisionSide::Left)
        : (requested > 0 ? CollisionSide::Down : CollisionSide::Up);
    // Keeps the closest contacts in the requested direction for callbacks after physics
    auto considerContact = [&](int gap, CollisionSide side, GameObject* other) {
        if (requested == 0 || side != contactSide || gap < 0) return;
        if (contactGap == -1 || gap < contactGap) {
            contactGap = gap;
            contactIds.clear();
        }
        if (gap == contactGap && (object->OnCollision || other->OnCollision)) {
            contactIds.push_back(other->objectId);
        }
    };
    auto index = this->GetGridIndex(object);
    auto objectRight = object->x + object->width;
    auto objectBottom = object->y + object->height;
    auto indexToCheck = std::vector<int>{0};
    if (index > 1) indexToCheck.push_back(index - 1);
    if (index != 0) indexToCheck.push_back(index);
    indexToCheck.push_back(index + 1);
    if (index == 0) {
        indexToCheck.clear();

        for (auto i = 0; i < this->grid.size(); ++i) {
            indexToCheck.push_back(static_cast<int>(i));
        }
    }
    for (auto idx: indexToCheck) {
        if (idx < 0 || idx >= this->grid.size()) continue;
        for (auto &obj: this->grid[idx]) {
            if (object->objectId == obj->objectId) continue;
            // A pusher may move through an entity horizontally so the push
            // can be resolved, but the entity is still a vertical collider.
            const bool pushingEntity = object->canPushEntities && obj->IsEntity();
            
            auto objLeft = obj->x;
            auto objRight = obj->x + obj->width;
            auto objTop = obj->y;
            auto objBottom = obj->y + obj->height;

            if (checkX && !pushingEntity) {
                bool yInBox = (object->y < objBottom && object->y + object->height > objTop);
                if (yInBox) {
                    auto moveRightRestricted = (object->moveRestricted & Direction::Right) == Direction::Right;
                    auto moveLeftRestricted = (object->moveRestricted & Direction::Left) == Direction::Left;
                    if (!moveRightRestricted) {
                        auto diffXR = objLeft - objectRight;
                        considerContact(diffXR, CollisionSide::Right, obj);
                        if (diffXR >= 0 && (diffXR < allowMoveXR || allowMoveXR == -1)) {
                            allowMoveXR = diffXR;
                        }
                    } else {
                        allowMoveXR = 0;
                    }

                    if (!moveLeftRestricted) {
                        auto diffXL = object->x - objRight;
                        considerContact(diffXL, CollisionSide::Left, obj);
                        if (diffXL >= 0 && (diffXL < allowMoveXL || allowMoveXL == -1)) {
                            allowMoveXL = diffXL;
                        }
                    } else {
                        allowMoveXL = 0;
                    }
                }
            }
            
            if (checkY) {
                bool xInBox = (object->x < objRight && object->x + object->width > objLeft);
                if (xInBox) {
                    auto moveUpRestricted = (object->moveRestricted & Direction::Up) == Direction::Up;
                    auto moveDownRestricted = (object->moveRestricted & Direction::Down) == Direction::Down;
                    if (!moveDownRestricted) {
                        auto diffYD = objTop - objectBottom;
                        considerContact(diffYD, CollisionSide::Down, obj);
                        if (diffYD >= 0 && (diffYD < allowMoveYD || allowMoveYD == -1)) {
                            allowMoveYD = diffYD;
                        }
                    } else {
                        allowMoveYD = 0;
                    }

                    if (!moveUpRestricted) {
                        auto diffYU = object->y - objBottom;
                        considerContact(diffYU, CollisionSide::Up, obj);
                        if (diffYU >= 0 && (diffYU < allowMoveYU || allowMoveYU == -1)) {
                            allowMoveYU = diffYU;
                        }
                    } else {
                        allowMoveYU = 0;
                    }
                }
            }
        }
    }

    // Check top and left
    if (object->x >= 0 && (object->x < allowMoveXL || allowMoveXL == -1)) {
        allowMoveXL = object->x;
    }
    if (object->y >= 0 && (object->y < allowMoveYU || allowMoveYU == -1)) {
        allowMoveYU = object->y;
    }

    const int allowed = contactSide == CollisionSide::Right ? allowMoveXR
        : contactSide == CollisionSide::Left ? allowMoveXL
        : contactSide == CollisionSide::Down ? allowMoveYD : allowMoveYU;
    if (requested != 0 && contactGap == allowed && allowed <= std::abs(requested)) {
        for (int otherId: contactIds) {
            collisionEvents.push_back({object->objectId, otherId, contactSide});
        }
    }

    ColliderRV rv;
    rv.allowMoveXL = allowMoveXL;
    rv.allowMoveXR = allowMoveXR;
    rv.allowMoveYU = allowMoveYU;
    rv.allowMoveYD = allowMoveYD;
    return rv;
}

// Checks movement for a temporary rectangle using the current collision grid
MovementInfo GameEngine::GetMovementInformationFromPoint(int x, int y, int width, int height, bool checkX, bool checkY, bool entity) {
    auto obj = GameObject(Color::Black(), true, true, 0);
    obj.Init(x, y, width, height, "!DUMMY!", entity);
    obj.objectId = -1;
    return MovementInfo(IsColliding(&obj, checkX, checkY));
}

// Checks movement at an offset from the object without moving the actual object
MovementInfo GameEngine::GetMovementInformationFromOffset(GameObject *object, int offsetX, int offsetY, bool checkX, bool checkY, bool entity) {
    return MovementInfo(this->GetMovementInformationFromPoint(object->x + offsetX, object->y + offsetY, object->width, object->height, checkX, checkY, entity));
}

// Finds nearby objects in the grid, optionally only entities.
// Returns: 0 = up, 1 = left, 2 = down, 3 = right
std::array<std::list<GameObject*>, 4> GameEngine::GetTouching(GameObject *object, bool checkEntity, int range) {
    if (!this->upToDateGrid) {
        this->UpdateGrid();
    }
    std::array<std::list<GameObject*>, 4> objects = { };
    auto index = this->GetGridIndex(object);
    auto objectRight = object->x + object->width;
    auto objectBottom = object->y + object->height;
    auto indexToCheck = std::vector<int>{0, index - 1, index, index + 1};
    if (index == 0) {
        indexToCheck.clear();

        for (auto i = 0; i < this->grid.size(); ++i) {
            indexToCheck.push_back(static_cast<int>(i));
        }
    }
    for (auto idx: indexToCheck) {
        if (idx < 0 || idx >= this->grid.size()) continue;
        for (auto &obj: this->grid[idx]) {
            if (object->objectId == obj->objectId || (checkEntity && !obj->IsEntity())) continue;
            
            auto objLeft = obj->x;
            auto objRight = obj->x + obj->width;
            auto objTop = obj->y;
            auto objBottom = obj->y + obj->height;

            bool yInBox = (object->y < objBottom && object->y + object->height > objTop);
            if (yInBox) {
                if (std::abs(objLeft - objectRight) <= range) {
                    objects[3].push_back(obj);
                }

                if (std::abs(object->x - objRight) <= range) {
                    objects[1].push_back(obj);
                }
            }
            
            bool xInBox = (object->x < objRight && object->x + object->width > objLeft);
            if (xInBox) {
                if (std::abs(objTop - objectBottom) <= range) {
                    objects[2].push_back(obj);
                }

                if (std::abs(object->y - objBottom) <= range) {
                    objects[0].push_back(obj);
                }
            }
        }
    }
    return objects;
}

// Gets nearby objects on each side, including objects that are not entities
std::array<std::list<GameObject*>, 4> GameEngine::GetAllNearbyObjects(GameObject *object, int range) {
    return this->GetTouching(object, false, range);
}

// Applies gravity and movement on each axis, then passes any pushing movement to nearby objects
void GameEngine::ProcessObject(GameObject* object) {
    object->TickVerticalMotion();

    auto allTouchingEntitiesX = this->GetAllNearbyObjects(object, std::abs(object->GetPendingX()));
    auto allTouchingEntitiesY = this->GetAllNearbyObjects(object, std::abs(object->GetPendingY()));

    const int pendingX = object->GetPendingX();
    auto canMove = this->IsColliding(object, true, false, pendingX);

    auto movedX = object->TickPendingX(canMove);

    const int pendingY = object->GetPendingY();
    canMove = this->IsColliding(object, false, true, pendingY);

    auto movedY = object->TickPendingY(canMove);

    if (allTouchingEntitiesX[0].size() > 0 && movedY > 0) {
        for (auto &entity: allTouchingEntitiesX[0]) {
            entity->ForceCanGoDown(false);
        }
    }

    if (allTouchingEntitiesX[2].size() > 0 && movedY < 0) {
        for (auto &entity: allTouchingEntitiesX[2]) {
            entity->ForceCanGoDown(true);
        }
    }

    if (object->canPushEntities) {
        if (movedX != 0) {
            if (movedX < 0) {
                ProcessPushMovement(allTouchingEntitiesX[1], false, Axis::X, movedX, object);
            } else {
                ProcessPushMovement(allTouchingEntitiesX[3], true, Axis::X, movedX, object);
            }
        }

        if (movedY != 0) {
            if (movedY < 0) {
                ProcessPushMovement(allTouchingEntitiesY[0], true, Axis::Y, movedY, object);
            } else {
                ProcessPushMovement(allTouchingEntitiesY[2], false, Axis::Y, movedY, object);
            }
        }
    }

    object->moveRestricted = Direction::None;
}

// Queues movement for objects being pushed and prevents them from moving back into the pusher.
// moveDirection is true for right on X and up on Y.
void GameEngine::ProcessPushMovement(std::list<GameObject*> entities, bool moveDirection, Axis axis, int movedValue, GameObject* object) {
    for (auto& i: entities) {
        auto pend = axis == Axis::X ? i->GetPendingX() : i->GetPendingY();
        auto tomove = movedValue - pend;
        if ((moveDirection && axis == Axis::X) || (!moveDirection && axis == Axis::Y)) {
            if (pend > movedValue) return;
        } else {
            if (pend < movedValue) return;
        }

        auto diff = 0.0f;

        if (axis == Axis::X) {
            if (moveDirection) {
                diff = std::abs(object->x + object->width - (tomove + pend + i->x));
                i->Move(tomove - diff, 0);
                i->moveRestricted |= Direction::Left;
            } else {
                diff = object->x - (tomove + pend + i->x + i->width);
                i->Move(tomove + diff, 0);
                i->moveRestricted |= Direction::Right;
            }
        } else {
            if (moveDirection) {
                diff = std::abs(object->y - (tomove + pend + i->y + i->height));
                i->Move(0, tomove + diff);
                i->moveRestricted |= Direction::Down;
            } else {
                diff = std::abs(object->y + object->height - (tomove + pend + i->y));
                i->Move(0, tomove - diff);
                i->moveRestricted |= Direction::Up;
            }
        }
    }
}

// Updates movable objects, then fires collision callbacks after movement is finished
void GameEngine::RunPhysics() {
    this->upToDateGrid = false;
    this->collisionEvents.clear();
    for (auto &i: this->objects) {
        if (!i.canMove) {
            continue;
        }
        this->ProcessObject(&i);
    }

    // Resolve by IDs - just in case one or the other is destroyed
    auto events = std::move(this->collisionEvents);
    for (const auto& event: events) {
        auto* object = GetObjectByObjectId(event.objectId);
        auto* other = GetObjectByObjectId(event.otherId);
        if (object && other && object->OnCollision) {
            auto callback = object->OnCollision;
            callback(*other, event.side);
        }
        object = GetObjectByObjectId(event.objectId);
        other = GetObjectByObjectId(event.otherId);
        if (object && other && other->OnCollision) {
            const auto opposite = event.side == CollisionSide::Up ? CollisionSide::Down
                : event.side == CollisionSide::Down ? CollisionSide::Up
                : event.side == CollisionSide::Left ? CollisionSide::Right : CollisionSide::Left;
            auto callback = other->OnCollision;
            callback(*object, opposite);
        }
    }
}

// Looks up an internal object ID, or returns nullptr if the object has been removed
GameObject* GameEngine::GetObjectByObjectId(int objectId) {
    auto it = objectsById.find(objectId);
    return it == objectsById.end() ? nullptr : it->second;
}

// Gets the number of objects currently stored in the engine
int GameEngine::GetGameObjectCount() {
    return this->objects.size();
}

// Runs one physics step and draws the result. Input is ticked separately.
void GameEngine::Tick() {
    this->StepPhysics();
    this->Render();
}

// Draws a frame, optionally running physics first so the game can pause movement
void GameEngine::Tick(bool runPhysics) {
    if (runPhysics) {
        this->StepPhysics();
    }
    this->Render();
}

// Runs one physics step without drawing a frame
void GameEngine::StepPhysics() {
    this->RunPhysics();
}

// Draws the current objects without updating physics
void GameEngine::Render() {
    this->drawing->RenderFrame(this->objects);
}

// Stores a copy of the object and assigns an internal ID for collision callbacks
void GameEngine::AddObject(GameObject object) {
    object.SetGameEngine(this);
    object.objectId = this->totalObjects;
    this->totalObjects++;
    this->objects.push_back(object);
    this->objectsById[object.objectId] = &this->objects.back();
    this->upToDateGrid = false;
}

// Removes all objects and clears the ID lookup. Existing object pointers are no longer valid.
void GameEngine::RemoveAllObjects() {
    this->objects.clear();
    this->objectsById.clear();
    this->upToDateGrid = false;
}

// Removes the first object with this string ID, or prints an error if it is missing
void GameEngine::RemoveObject(std::string id) {
    for (auto it = this->objects.begin(); it != this->objects.end(); ++it) {
        if (it->id == id) {
            this->objectsById.erase(it->objectId);
            this->objects.erase(it);
            this->upToDateGrid = false;
            return;
        }
    }
    std::cerr << "Tried to remove object not in GameEngine! Object ID: " << id << std::endl;
}

// Removes an object using its string ID. The supplied pointer must be valid.
void GameEngine::RemoveObject(GameObject *object) {
    this->RemoveObject(object->id);
}

// Gets the first object with this string ID, or nullptr if it was not found
GameObject* GameEngine::GetObjectRef(std::string id) {
    for (auto& i: this->objects) {
        if (i.id == id) {
            return &i;
        }
    }
    //std::cout << "Object not found: " << id << std::endl;
    return nullptr;
}

// Checks grid candidates against a screen point and returns the last match, or nullptr.
// Only objects in the collision grid are checked; points on the edges do not count.
GameObject* GameEngine::GetObjectRefAtPoint(int32_t x, int32_t y) {
    if (!this->upToDateGrid) {
        this->UpdateGrid();
    }
    auto index = this->GetGridIndex(x);
    auto indexToCheck = std::vector<int>{0, index - 1, index, index + 1};
    GameObject* found = nullptr;
    for (auto idx: indexToCheck) {
        if (idx < 0 || idx >= this->grid.size()) continue;
        for (auto &obj: this->grid[idx]) {
            
            auto objLeft = this->camera.GetX(obj->x);
            auto objRight = this->camera.GetX(obj->x + obj->width);
            auto objTop = this->camera.GetY(obj->y);
            auto objBottom = this->camera.GetY(obj->y + obj->height);

            bool isInBox = (y < objBottom && y > objTop && x < objRight && x > objLeft);
            if (isInBox) {
                found = obj;
            }
        }
    }
    return found;
}

// Gets pointers to all objects whose IDs start with the supplied text
std::list<GameObject*> GameEngine::GetObjectsRefWithPrefix(std::string id) {
    std::list<GameObject*> result;
    for (auto& i: this->objects) {
        if (i.id.rfind(id) == 0) {
            result.push_back(&i);
        }
    }
    return result;
}

// Gets the first object whose ID starts with the supplied text, or nullptr
GameObject* GameEngine::GetFirstObjectRefWithPrefix(std::string id) {
    for (auto& i: this->objects) {
        if (i.id.rfind(id) == 0) {
            return &i;
        }
    }
    return nullptr;
}

// Gets objects whose IDs contain the supplied text. This currently matches anywhere, not just the suffix.
std::list<GameObject*> GameEngine::GetObjectsRefWithSuffix(std::string id) {
    std::list<GameObject*> result;
    for (auto& i: this->objects) {
        if (i.id.find(id) != std::string::npos) {
            result.push_back(&i);
        }
    }
    return result;
}

// Removes all objects whose IDs start with the supplied text and updates the ID lookup
void GameEngine::RemoveObjectsWithIdPrefix(std::string id) {
    for (auto it = this->objects.begin(); it != this->objects.end(); ) {
        if (it->id.rfind(id) == 0) {
            this->objectsById.erase(it->objectId);
            it = this->objects.erase(it);
            this->upToDateGrid = false;
        } else {
            ++it;
        }
    }
}

// Gets the width and height currently stored by the drawing engine
WindowSize GameEngine::GetWindowSize() {
    return this->drawing->GetWindowSize();
}
