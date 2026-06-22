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
    Item content;
    bool isOpen;
    int uniqueID;
};

#define MAX_PORTALS 20 
struct Portal {
    Rectangle  bounds;
    std::string targetMap;
    float       spawnX;
    float       spawnY;
    bool        requiresKey;   // If true, player must hold a Key to enter
    int         requiredItemID;
};

#define MAX_GATES 20
struct Gate {
    Rectangle bounds;
    int uniqueID;
    bool isLocked;
    int requiredItemID;
};

#define MAX_SIGNPOSTS 10
#define MAX_LINES_PER_SIGNPOST 5
struct Signpost {
    Rectangle bounds;
    std::string dialogue[MAX_LINES_PER_SIGNPOST];
    int lineCount;
};

#define MAX_ENEMIES 10
#define MAX_ENEMY_HISTORY 100
struct Enemy {
    Rectangle bounds;
    int uniqueID;
    int typeID;


    // Enemies Stat
    std::string name; 
    int maxHp;        
    int hp;           
    int attack;
    float speed;
    int aggroRange;
    bool isAggro;
    bool isDefeated;
    Item lootDrop;
    bool hasLoot;
    int expReward;
    int scoreReward;

    float spawnX;
    float spawnY;
};

struct Point2D {
    int x;
    int y;
};

class GameMap {
private:
    int grid[MAP_ROWS][MAP_COLS];
    Texture2D wallSprite; 
    Texture2D portalSprite;
    Texture2D portalLockedSprite;
    Texture2D gateSprite;
    Texture2D gateLockedSprite;
    Texture2D chestClosedSprite; 
    Texture2D chestOpenSprite;
    Texture2D signSprite;
    
    Texture2D NULLByteSprite;
    Texture2D LostArraySprite;
    Texture2D CompilerSprite;
    
    Portal portals[MAX_PORTALS];
    std::string unlockedPortals[50];
    int portalCount;      
    int unlockedPortalCount;

    Gate gates[MAX_GATES];
    int gateCount;
    int unlockedGates[100]; 
    int unlockedGateCount;

    Chest chests[MAX_CHESTS];
    int chestCount;
    int openedHistory[MAX_HISTORY]; 
    int historyCount;

    Signpost signposts[MAX_SIGNPOSTS];
    int signpostCount;

    Enemy enemies[MAX_ENEMIES];
    int enemyCount;
    int defeatedHistory[MAX_ENEMY_HISTORY];
    int defeatedCount;
    bool isAggro;

public:
    GameMap();
    ~GameMap(); 
    
    bool LoadMap(const std::string& filename); 
    bool IsSolid(int targetX, int targetY);
    bool CheckCollision(Rectangle rect);
    void ResetProgress();
    void Draw();
    
    // Default spawn coordinate
    float defaultSpawnX = 0.0f;
    float defaultSpawnY = 0.0f;

    // requiresKey defaults to false for backward compatibility
    void AddPortal(Rectangle bounds, std::string targetMap,
                   float spawnX, float spawnY, bool requiresKey = false, int requiredItemID = 0);
    Portal* CheckPortals(Rectangle playerBounds);
    void MarkPortalUnlocked(const std::string& targetMap);

    Gate* CheckGateInteraction(Rectangle playerBounds);
    void MarkGateUnlocked(Gate* gate);

    void AddChest(Rectangle bounds, Item content);
    Chest*    CheckChestInteraction(Rectangle playerBounds);
    void      MarkChestOpened(Chest* chest);

    Signpost* CheckSignpostInteraction(Rectangle playerBounds);

    void    UpdateEnemies(Rectangle playerBounds);
    Enemy*  CheckEnemyCollision(Rectangle playerBounds);
    void    MarkEnemyDefeated(Enemy* enemy);
    Point2D GetNextPathStep(int startX, int startY, int targetX, int targetY);
    bool HasLineOfSight(int x0, int y0, int x1, int y1);
    void    ResetDefeatedEnemies();

    Item GetEnemyLoot(Enemy* enemy);
};

#endif
