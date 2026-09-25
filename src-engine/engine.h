#pragma once
#include <list>
#include <array>
#include <unordered_map>
#include "drawing.h"
#include "gameobject.h"
#include "collider.h"
#include "input/input.h"

class GameObject;
struct WindowSize;
class DrawingEngine;

enum class Axis {
    X,
    Y
};

struct Camera {
    int32_t x;
    int32_t y;
    
    // Converts a world X coordinate to a screen X coordinate
    int32_t GetX(int32_t x) { return x - this->x; }
    // Converts a world Y coordinate to a screen Y coordinate
    int32_t GetY(int32_t y) { return y - this->y; }
    // Moves the camera back to the world origin
    void Reset() { x = 0; y = 0; }
    
    // Creates a camera at the supplied world position
    Camera(int32_t x = 0, int32_t y = 0) :
        x(x),
        y(y)
    {}
};

class GameEngine {
private:
    DrawingEngine *drawing;

    std::list<GameObject> objects{};
    std::unordered_map<int, GameObject*> objectsById;
    
    int gridBlockWidth = 100;
    std::vector<std::vector<GameObject*>> grid;
    void UpdateGrid();
    int GetGridIndex(GameObject* obj);
    int GetGridIndex(int x);
    bool upToDateGrid = false;
    bool entityCollisions = true;
    int totalObjects = 0;
    struct CollisionEvent {
        int objectId;
        int otherId;
        CollisionSide side;
    };
    std::vector<CollisionEvent> collisionEvents;
    GameObject* GetObjectByObjectId(int objectId);
    
    // Reserved drawing helper. This currently has no implementation.
    void DrawObject(GameObject obj);
    void RunPhysics();
    void ProcessObject(GameObject* object);
    void ProcessPushMovement(std::list<GameObject*> entities, bool moveDirection, Axis axis, int movedValue, GameObject* object);
    ColliderRV IsColliding(GameObject *object, bool checkX, bool checkY, int requested = 0);
    std::array<std::list<GameObject*>, 4> GetTouching(GameObject *object, bool checkEntity, int range);
    
    
public:
    GameEngine();
    ~GameEngine();

    InputDriver input;
    Camera camera;

    void Tick();
    void Tick(bool runPhysics);
    void StepPhysics();
    void Render();

    void AddObject(GameObject object);
    void RemoveObject(std::string id);
    void RemoveObject(GameObject *object);
    void RemoveAllObjects();
    // Reserved update helper. This currently has no implementation.
    void UpdateObjects();
    void SetEntityCollisions(bool check);
    int GetGameObjectCount();
    void ResizeWindow();
    // Currently not implemented. Use camera.Reset() to reset the camera.
    void ResetCamera();
    
    WindowSize GetWindowSize();

    GameObject* GetObjectRef(std::string id);
    GameObject* GetObjectRefAtPoint(int32_t x, int32_t y);
    std::list<GameObject*> GetObjectsRefWithPrefix(std::string id);
    std::list<GameObject*> GetObjectsRefWithSuffix(std::string id);
    GameObject* GetFirstObjectRefWithPrefix(std::string id);
    void RemoveObjectsWithIdPrefix(std::string id);

    std::array<std::list<GameObject*>, 4> GetAllNearbyObjects(GameObject *object, int range);

    MovementInfo GetMovementInformationFromPoint(int x, int y, int width, int height, bool checkX, bool checkY, bool entity);
    MovementInfo GetMovementInformationFromOffset(GameObject *object, int offsetX, int offsetY, bool checkX, bool checkY, bool entity);
};
