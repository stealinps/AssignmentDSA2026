#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include "config.h"
#include <string> 

#define MAP_COLS (SCREEN_WIDTH / TILE_SIZE)
#define MAP_ROWS (SCREEN_HEIGHT / TILE_SIZE)

#define MAX_CHESTS 10
#define MAX_HISTORY 100
struct Chest {
    Rectangle bounds;
    Item content;      // What is inside the chest?
    bool isOpen;       // Has the player looted it already?
    int uniqueID;
};

// Define a reasonable limit for maximum doors on one single screen
#define MAX_PORTALS 20 
struct Portal {
    Rectangle bounds;      
    std::string targetMap; 
    float spawnX;          
    float spawnY;
};

#define MAX_SIGNPOSTS 10
#define MAX_LINES_PER_SIGNPOST 5
struct Signpost {
    Rectangle bounds;
    std::string dialogue[MAX_LINES_PER_SIGNPOST];
    int lineCount;
};

class GameMap {
private:
    int grid[MAP_ROWS][MAP_COLS];
    Texture2D wallSprite; 
    Texture2D portalSprite;
    Texture2D chestClosedSprite; 
    Texture2D chestOpenSprite;
    Texture2D signSprite;
    
    Portal portals[MAX_PORTALS]; 
    int portalCount;      

    Chest chests[MAX_CHESTS];
    int chestCount;
    int openedHistory[MAX_HISTORY]; 
    int historyCount;

    Signpost signposts[MAX_SIGNPOSTS];
    int signpostCount;

public:
    GameMap();
    ~GameMap(); 
    
    bool LoadMap(const std::string& filename); 
    bool IsSolid(int targetX, int targetY);
    bool CheckCollision(Rectangle rect);
    void Draw();

    void AddPortal(Rectangle bounds, std::string targetMap, float spawnX, float spawnY);
    bool CheckPortals(Rectangle playerBounds, Portal& outPortal);

    void AddChest(Rectangle bounds, Item content);
    
    // Returns a POINTER (memory address) to a chest
    Chest* CheckChestInteraction(Rectangle playerBounds);
    void MarkChestOpened(Chest* chest);

    Signpost* CheckSignpostInteraction(Rectangle playerBounds);
};

#endif