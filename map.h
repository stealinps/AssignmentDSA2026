#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include "config.h"
#include <string> 

#define MAP_COLS (SCREEN_WIDTH / TILE_SIZE)
#define MAP_ROWS (SCREEN_HEIGHT / TILE_SIZE)

// Define a reasonable limit for maximum doors on one single screen
#define MAX_PORTALS 20 

struct Portal {
    Rectangle bounds;      
    std::string targetMap; 
    float spawnX;          
    float spawnY;
};

class GameMap {
private:
    int grid[MAP_ROWS][MAP_COLS];
    Texture2D wallSprite; 
    Texture2D portalSprite;
    
    Portal portals[MAX_PORTALS]; 
    int portalCount;             

public:
    GameMap();
    ~GameMap(); 
    
    bool LoadMap(const std::string& filename); 
    bool IsSolid(int targetX, int targetY);
    bool CheckCollision(Rectangle rect);
    void Draw();

    void AddPortal(Rectangle bounds, std::string targetMap, float spawnX, float spawnY);
    bool CheckPortals(Rectangle playerBounds, Portal& outPortal);
};

#endif