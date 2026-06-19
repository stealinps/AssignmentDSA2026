#include "map.h"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <raylib.h>
 
GameMap::GameMap() {
    for (int y = 0; y < MAP_ROWS; y++)
        for (int x = 0; x < MAP_COLS; x++)
            grid[y][x] = 0;
 
    wallSprite        = LoadTexture("src/sprite/wall.png");
    portalSprite      = LoadTexture("src/sprite/door.png");
    portalLockedSprite= LoadTexture("src/sprite/door_locked.png");
    gateSprite   = LoadTexture("src/sprite/gate.png");
    gateLockedSprite = LoadTexture("src/sprite/gate_locked.png");
    chestClosedSprite = LoadTexture("src/sprite/chest_closed.png");
    chestOpenSprite   = LoadTexture("src/sprite/chest_open.png");
    signSprite        = LoadTexture("src/sprite/sign.png");

    NULLByteSprite = LoadTexture("src/sprite/nullbyte.png");
    LostArraySprite = LoadTexture("src/sprite/lostarray.png");
    CompilerSprite = LoadTexture("src/sprite/compiler.png");
 
    portalCount   = 0;
    unlockedPortalCount = 0;   // FIX: was never initialized -> garbage value made
                               // the portal-unlock loop read out of bounds and
                               // segfault on Windows (Linux happened to read 0).
    chestCount    = 0;
    historyCount  = 0;
    signpostCount = 0;
    enemyCount    = 0;
    defeatedCount = 0;

    gateCount     = 0;
    unlockedGateCount = 0;

    isAggro       = false;
 
    std::srand(std::time(nullptr));
}
 
GameMap::~GameMap() {
    UnloadTexture(wallSprite);
    UnloadTexture(portalSprite);
    UnloadTexture(chestClosedSprite);
    UnloadTexture(chestOpenSprite);
    UnloadTexture(signSprite);
    UnloadTexture(gateSprite);
    UnloadTexture(gateLockedSprite);
    UnloadTexture(NULLByteSprite);
    UnloadTexture(LostArraySprite);
    UnloadTexture(CompilerSprite);
}
 
// Helper: fill in description and ensure quantity=1 for a given item ID.
// BUG FIX: The old loader wrote {itemID, itemName} which left quantity=0,
// making all chest loot unusable (DecreaseQuantity refused qty < 1).
static void FillItemData(Item& item) {
    if (item.quantity < 1) item.quantity = 1;
    if (!item.description.empty()) return; // Already set by caller
 
    switch (item.id) {
        case ITEM_HEALTH_POTION:
            item.description = "Restores 20 HP in battle";
            break;
        case ITEM_STRENGTH_POTION:
            item.description = "Deal +10 damage for 2 turns";
            break;
        case ITEM_DEFENSE_POTION:
            item.description = "Gain +50 max HP";
            break;
        case ITEM_IRON_KEY:
            item.description = "Opens locked doors";
            break;
        case ITEM_MASTER_KEY:
            item.description = "Debug master key - opens the test room";
            break;
        default:
            item.description = "A mysterious item";
            break;
    }
}

static void FillEnemyData(Enemy& enemy) {
    switch (enemy.typeID) {
        case 1:
            enemy.name = "Green Slime";
            enemy.maxHp = 20;
            enemy.hp = 20;
            enemy.attack = 5;
            enemy.speed = 50.0f;
            enemy.aggroRange = 100;
            enemy.hasLoot = true;
            enemy.lootDrop = {ITEM_HEALTH_POTION, "Health Potion", "Restores 20 HP", 1};
            break;
            
        case 2:
            enemy.name = "Lost Array";
            enemy.maxHp = 45;
            enemy.hp = 45;
            enemy.attack = 12;
            enemy.speed = 80.0f;
            enemy.aggroRange = 150;
            enemy.hasLoot = true;
            enemy.lootDrop = {ITEM_STRENGTH_POTION, "Strength Potion", "Deal +10 damage", 1};
            break;

        case 999: // Final Boss
            enemy.name = "Compiler";
            enemy.maxHp = 150;
            enemy.hp = 150;
            enemy.attack = 25;
            enemy.speed = 110.0f;
            enemy.aggroRange = 250;
            enemy.hasLoot = false;
            break;

        default: // Fallback if you type a wrong ID in the text file
            enemy.name = "Unknown Glitch";
            enemy.maxHp = 1;
            enemy.hp = 1;
            enemy.attack = 1;
            enemy.speed = 0.0f;
            enemy.aggroRange = 0;
            break;
    }
}
 
bool GameMap::LoadMap(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "ERROR: Could not load map file: " << filename << std::endl;
        return false;
    }
 
    // Clear old data — avoids duplicates across level transitions
    portalCount   = 0;
    chestCount    = 0;
    signpostCount = 0;
    enemyCount    = 0;
    gateCount     = 0;
 
    std::string token;
    
    
    while (file >> token) {
        
        // MAP
        if (token == "GRID") {
            for (int y = 0; y < MAP_ROWS; y++) {
                for (int x = 0; x < MAP_COLS; x++) {
                    file >> grid[y][x];
                }
            }
        }

        // PORTALS
        else if (token == "PORTAL") {
            int gridX, gridY, spawnGridX, spawnGridY, reqKeyID;
            std::string target;
            std::string lockStatus; // Used to get portal status (Locked or Unlocked)
            
            file >> gridX >> gridY >> target >> spawnGridX >> spawnGridY >> lockStatus >> reqKeyID;
            
            bool requiresKey = (lockStatus == "LOCKED");

            // Unlock logic override (If player already unlocked it before)
            for (int j = 0; j < unlockedPortalCount; j++) {
                if (unlockedPortals[j] == target) {
                    requiresKey = false; 
                    break;
                }
            }
 
            float px = (float)(gridX * TILE_SIZE);
            float py = (float)(gridY * TILE_SIZE);
            float sx = (float)(spawnGridX * TILE_SIZE);
            float sy = (float)(spawnGridY * TILE_SIZE);
 
            AddPortal({px, py, (float)TILE_SIZE, (float)TILE_SIZE}, target, sx, sy, requiresKey, reqKeyID);
        }

        // ---- GATES ----
        else if (token == "GATE") {
            int gridX, gridY, gateID, reqKeyID;
            file >> gridX >> gridY >> gateID >> reqKeyID;

            Gate newGate;
            newGate.bounds = {
                (float)(gridX * TILE_SIZE), 
                (float)(gridY * TILE_SIZE), 
                (float)TILE_SIZE, 
                (float)TILE_SIZE
            };
            newGate.uniqueID = gateID;
            newGate.isLocked = true; // Assume locked by default
            newGate.requiredItemID = reqKeyID;

            // Check memory to see if we already unlocked it
            for (int j = 0; j < unlockedGateCount; j++) {
                if (unlockedGates[j] == gateID) {
                    newGate.isLocked = false;
                    break;
                }
            }

            if (gateCount < MAX_GATES) gates[gateCount++] = newGate;
        }
        
        // CHESTS
        else if (token == "CHEST") {
            int gridX, gridY, chestID, itemID;
            std::string itemName;
            file >> gridX >> gridY >> chestID >> itemID >> itemName;
 
            for (int j = 0; j < itemName.length(); j++) {
                if (itemName[j] == '_') itemName[j] = ' ';
            }
 
            Chest newChest;
            newChest.bounds   = {(float)(gridX * TILE_SIZE), (float)(gridY * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE};
            newChest.uniqueID = chestID;
            newChest.isOpen   = false;
            newChest.content  = {itemID, itemName, "", 1};
            FillItemData(newChest.content);
 
            for (int j = 0; j < historyCount; j++) {
                if (openedHistory[j] == chestID) {
                    newChest.isOpen       = true;
                    newChest.content.id   = 0;
                    break;
                }
            }
 
            if (chestCount < MAX_CHESTS) chests[chestCount++] = newChest;
        }

        // ENEMIES
        else if (token == "ENEMY") {
            int gridX, gridY, instanceID, typeID;
            float speed;
            file >> gridX >> gridY >> instanceID >> typeID;
 
            float px = (float)(gridX * TILE_SIZE);
            float py = (float)(gridY * TILE_SIZE);
            float offsetX = (TILE_SIZE - 20.0f) / 2.0f;
            float offsetY = TILE_SIZE - 20.0f;
 
            Enemy newEnemy;
            newEnemy.bounds   = {px + offsetX, py + offsetY, 20.0f, 20.0f};
            newEnemy.uniqueID = instanceID;
            newEnemy.typeID   = typeID;
            newEnemy.spawnX   = px + offsetX;
            newEnemy.spawnY   = py + offsetY;
            newEnemy.isDefeated = false;

            FillEnemyData(newEnemy);
 
            /* int roll = rand() % 100;
            if (roll < 50) newEnemy.lootDrop = {ITEM_STRENGTH_POTION, "Strength Potion", "Deal +10 damage for 2 turns", 1};
            else if (roll < 80) newEnemy.lootDrop = {ITEM_DEFENSE_POTION, "Defense Potion", "Gain +50 max HP", 1};
            else newEnemy.lootDrop = {ITEM_IRON_KEY, "Iron Key", "Opens locked portals and doors", 1}; */
 
            for (int j = 0; j < defeatedCount; j++) {
                if (defeatedHistory[j] == instanceID) {
                    newEnemy.isDefeated = true;
                    newEnemy.hasLoot    = false;
                    break;
                }
            }
 
            if (enemyCount < MAX_ENEMIES) enemies[enemyCount++] = newEnemy;
        }

        // SIGNPOSTS
        else if (token == "SIGNPOST") {
            int gridX, gridY, numLines;
            file >> gridX >> gridY >> numLines;
 
            Signpost newSign;
            newSign.bounds    = {(float)(gridX * TILE_SIZE), (float)(gridY * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE};
            newSign.lineCount = 0;
 
            std::string dummy;
            std::getline(file, dummy); // Eat the invisible newline character
 
            for (int j = 0; j < numLines; j++) {
                std::string line;
                std::getline(file, line);
                if (newSign.lineCount < MAX_LINES_PER_SIGNPOST) {
                    newSign.dialogue[newSign.lineCount++] = line;
                }
            }
            if (signpostCount < MAX_SIGNPOSTS) signposts[signpostCount++] = newSign;
        }

        // SPAWN COORDINATES
        else if (token == "SPAWN") {
            int gridX, gridY;
            file >> gridX >> gridY;
            
            // The engine automatically converts your easy grid numbers into ugly pixel numbers
            defaultSpawnX = (float)(gridX * TILE_SIZE);
            defaultSpawnY = (float)(gridY * TILE_SIZE);
        }
    }
 
    file.close();
    return true;
}
 
// -------------------------------------------------------------------
// PORTALS
// -------------------------------------------------------------------
void GameMap::AddPortal(Rectangle bounds, std::string targetMap,
                        float spawnX, float spawnY, bool requiresKey, int requiredItemID) {
    if (portalCount >= MAX_PORTALS) {
        std::cout << "WARNING: Max portals reached." << std::endl;
        return;
    }
    Portal p;
    p.bounds      = bounds;
    p.targetMap   = targetMap;
    p.spawnX      = spawnX;
    p.spawnY      = spawnY;
    p.requiresKey = requiresKey;
    p.requiredItemID = requiredItemID;
    portals[portalCount++] = p;
}
 
Portal* GameMap::CheckPortals(Rectangle playerBounds) {
    for (int i = 0; i < portalCount; i++) {
        if (CheckCollisionRecs(playerBounds, portals[i].bounds)) {
            return &portals[i];
        }
    }
    return nullptr;
}
 
// -------------------------------------------------------------------
// DRAW
// -------------------------------------------------------------------
void GameMap::Draw() {
    // Walls
    for (int y = 0; y < MAP_ROWS; y++)
        for (int x = 0; x < MAP_COLS; x++)
            if (grid[y][x] == 1)
                DrawTexture(wallSprite, x * TILE_SIZE, y * TILE_SIZE, WHITE);
 
    // Portals
    for (int i = 0; i < portalCount; i++) {
        int drawX = (int)portals[i].bounds.x;
        int drawY = (int)portals[i].bounds.y;
        if (portals[i].requiresKey) {
            DrawTexture(portalLockedSprite, drawX, drawY, WHITE);
        } else {
            DrawTexture(portalSprite, drawX, drawY, WHITE);
        }
    }

    // Gates
    for (int i = 0; i < gateCount; i++) {
        int drawX = (int)gates[i].bounds.x;
        int drawY = (int)gates[i].bounds.y;
        if (gates[i].isLocked) {
            DrawTexture(gateLockedSprite, drawX, drawY, WHITE);
        } else {
            DrawTexture(gateSprite, drawX, drawY, WHITE);
        }
    }
 
    // Chests
    for (int i = 0; i < chestCount; i++) {
        int drawX = (int)chests[i].bounds.x;
        int drawY = (int)chests[i].bounds.y;
        DrawTexture(chests[i].isOpen ? chestOpenSprite : chestClosedSprite,
                    drawX, drawY, WHITE);
    }
 
    // Signposts
    for (int i = 0; i < signpostCount; i++)
        DrawTexture(signSprite,
                    (int)signposts[i].bounds.x,
                    (int)signposts[i].bounds.y, WHITE);
 
    // Enemies
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].isDefeated) continue;
        int drawX = (int)(enemies[i].bounds.x - 6.0f);
        int drawY = (int)(enemies[i].bounds.y - 12.0f);

        Texture2D currentSprite;
        switch (enemies[i].typeID) {
            case 1:  currentSprite = NULLByteSprite; break;
            case 2:  currentSprite = LostArraySprite; break;
            case 999:  currentSprite = CompilerSprite; break;
            default: currentSprite = NULLByteSprite; break; // Fallback just in case
        }

        DrawTexture(currentSprite, drawX, drawY, isAggro ? RED : WHITE);
        // DrawRectangleLinesEx(enemies[i].bounds, 1, RED); // Debug hitbox
    }
}
 
// -------------------------------------------------------------------
// COLLISION
// -------------------------------------------------------------------
bool GameMap::IsSolid(int targetX, int targetY) {
    if (targetX < 0 || targetX >= MAP_COLS ||
        targetY < 0 || targetY >= MAP_ROWS) return true;
    return grid[targetY][targetX] == 1;
}
 
bool GameMap::CheckCollision(Rectangle rect) {
    // Wall Collision
    int startCol = (int)(rect.x / TILE_SIZE);
    int endCol   = (int)((rect.x + rect.width)  / TILE_SIZE);
    int startRow = (int)(rect.y / TILE_SIZE);
    int endRow   = (int)((rect.y + rect.height) / TILE_SIZE);
    for (int row = startRow; row <= endRow; row++)
        for (int col = startCol; col <= endCol; col++)
            if (IsSolid(col, row)) return true;

    // Gate Collision
    for (int i = 0; i < gateCount; i++) {
        // If the gate is locked AND the player's rectangle hits it, block them!
        if (gates[i].isLocked && CheckCollisionRecs(rect, gates[i].bounds)) {
            return true; 
        }
    }

    return false;
}
 
// -------------------------------------------------------------------
// CHESTS
// -------------------------------------------------------------------
void GameMap::AddChest(Rectangle bounds, Item content) {
    if (chestCount >= MAX_CHESTS) return;
    FillItemData(content); // Ensure description and quantity are set
    Chest c;
    c.bounds  = bounds;
    c.content = content;
    c.isOpen  = false;
    chests[chestCount++] = c;
}
 
Chest* GameMap::CheckChestInteraction(Rectangle playerBounds) {
    Rectangle reach = {
        playerBounds.x - 10,
        playerBounds.y - 10,
        playerBounds.width  + 20,
        playerBounds.height + 20
    };
    for (int i = 0; i < chestCount; i++)
        if (CheckCollisionRecs(reach, chests[i].bounds))
            return &chests[i];
    return nullptr;
}
 
void GameMap::MarkChestOpened(Chest* chest) {
    chest->isOpen     = true;
    chest->content.id = 0;
    if (historyCount < MAX_HISTORY) {
        openedHistory[historyCount++] = chest->uniqueID;
        std::cout << "[MEMORY] Saved Chest ID: " << chest->uniqueID
                  << " | Total opened: " << historyCount << std::endl;
    }
}
 
// -------------------------------------------------------------------
// SIGNPOSTS
// -------------------------------------------------------------------
Signpost* GameMap::CheckSignpostInteraction(Rectangle playerBounds) {
    Rectangle reach = {
        playerBounds.x - 10, playerBounds.y - 10,
        playerBounds.width + 20, playerBounds.height + 20
    };
    for (int i = 0; i < signpostCount; i++)
        if (CheckCollisionRecs(reach, signposts[i].bounds))
            return &signposts[i];
    return nullptr;
}
 
// -------------------------------------------------------------------
// ENEMIES
// -------------------------------------------------------------------
void GameMap::UpdateEnemies(Rectangle playerBounds) {
    float deltaTime = GetFrameTime();
    isAggro = false; // Reset per frame; set true if any enemy is chasing
 
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].isDefeated) continue;
 
        float pCX = playerBounds.x + playerBounds.width  / 2;
        float pCY = playerBounds.y + playerBounds.height / 2;
        float eCX = enemies[i].bounds.x + enemies[i].bounds.width  / 2;
        float eCY = enemies[i].bounds.y + enemies[i].bounds.height / 2;
 
        float dx    = pCX - eCX;
        float dy    = pCY - eCY;
        float dist  = sqrtf(dx * dx + dy * dy);
 
        if (dist < enemies[i].aggroRange) {
            isAggro = true;
 
            int eGX = (int)(eCX / TILE_SIZE);
            int eGY = (int)(eCY / TILE_SIZE);
            int pGX = (int)(pCX / TILE_SIZE);
            int pGY = (int)(pCY / TILE_SIZE);
 
            Point2D next = GetNextPathStep(eGX, eGY, pGX, pGY);
            float tPX = next.x * TILE_SIZE + TILE_SIZE / 2.0f;
            float tPY = next.y * TILE_SIZE + TILE_SIZE / 2.0f;
            float sDX = tPX - eCX;
            float sDY = tPY - eCY;
            float sD  = sqrtf(sDX * sDX + sDY * sDY);
 
            if (sD > 2.0f) {
                float dirX = sDX / sD;
                float dirY = sDY / sD;
                enemies[i].bounds.x += dirX * enemies[i].speed * deltaTime;
                if (CheckCollision(enemies[i].bounds))
                    enemies[i].bounds.x -= dirX * enemies[i].speed * deltaTime;
                enemies[i].bounds.y += dirY * enemies[i].speed * deltaTime;
                if (CheckCollision(enemies[i].bounds))
                    enemies[i].bounds.y -= dirY * enemies[i].speed * deltaTime;
            }
        } else {
            // Walk back to spawn at half speed
            float hDX   = enemies[i].spawnX - enemies[i].bounds.x;
            float hDY   = enemies[i].spawnY - enemies[i].bounds.y;
            float hDist = sqrtf(hDX * hDX + hDY * hDY);
            if (hDist > 1.0f) {
                float dirX = hDX / hDist;
                float dirY = hDY / hDist;
                float retS = enemies[i].speed * 0.5f;
 
                enemies[i].bounds.x += dirX * retS * deltaTime;
                if (CheckCollision(enemies[i].bounds))
                    enemies[i].bounds.x -= dirX * retS * deltaTime;
                enemies[i].bounds.y += dirY * retS * deltaTime;
                if (CheckCollision(enemies[i].bounds))
                    enemies[i].bounds.y -= dirY * retS * deltaTime;
            }
        }
    }
}
 
Enemy* GameMap::CheckEnemyCollision(Rectangle playerBounds) {
    for (int i = 0; i < enemyCount; i++)
        if (!enemies[i].isDefeated &&
            CheckCollisionRecs(playerBounds, enemies[i].bounds))
            return &enemies[i];
    return nullptr;
}
 
void GameMap::MarkEnemyDefeated(Enemy* enemy) {
    if (enemy == nullptr) return;
    enemy->isDefeated = true;
    if (defeatedCount < MAX_ENEMY_HISTORY)
        defeatedHistory[defeatedCount++] = enemy->uniqueID;
}
 
Item GameMap::GetEnemyLoot(Enemy* enemy) {
    if (enemy != nullptr && enemy->hasLoot) {
        enemy->hasLoot = false;
        return enemy->lootDrop;
    }
    return Item{0, "EMPTY", "No loot", 0};
}
 
void GameMap::ResetDefeatedEnemies() {
    defeatedCount = 0;
    for (int i = 0; i < enemyCount; i++) {
        enemies[i].isDefeated = false;
        enemies[i].bounds.x   = enemies[i].spawnX;
        enemies[i].bounds.y   = enemies[i].spawnY;
        enemies[i].hasLoot    = true;
    }
}
 
// -------------------------------------------------------------------
// BFS PATHFINDER
// -------------------------------------------------------------------
Point2D GameMap::GetNextPathStep(int startX, int startY, int targetX, int targetY) {
    // GUARD CLAUSE, START AND TARGET WONT BE AT THE SAME LOCATION
    if (startX == targetX && startY == targetY) {
        return {startX, startY};
    }
    Point2D bfsQueue[MAP_ROWS * MAP_COLS];
    int front = 0, back = 0;

 
    bool    visited[MAP_ROWS][MAP_COLS] = {};
    Point2D parent [MAP_ROWS][MAP_COLS];
 
    bfsQueue[back++]       = {startX, startY};
    visited[startY][startX] = true;
    parent [startY][startX] = {-1, -1};
 
    const int dx[] = {0, 0, -1, 1};
    const int dy[] = {-1, 1,  0, 0};
    bool found = false;
 
    while (front < back) {
        Point2D cur = bfsQueue[front++];
        if (cur.x == targetX && cur.y == targetY) { found = true; break; }
        for (int i = 0; i < 4; i++) {
            int nx = cur.x + dx[i];
            int ny = cur.y + dy[i];
            if (nx >= 0 && nx < MAP_COLS && ny >= 0 && ny < MAP_ROWS &&
                grid[ny][nx] != 1 && !visited[ny][nx]) {
                visited[ny][nx]   = true;
                parent [ny][nx]   = cur;
                bfsQueue[back++]  = {nx, ny};
            }
        }
    }
 
    if (!found) return {startX, startY};
 
    Point2D step = {targetX, targetY};
    while (parent[step.y][step.x].x != startX ||
           parent[step.y][step.x].y != startY) {
        step = parent[step.y][step.x];
    }
    return step;
}

void GameMap::ResetProgress() {
    historyCount = 0;
    defeatedCount = 0;
    unlockedPortalCount = 0;
    unlockedGateCount = 0;
}

void GameMap::MarkPortalUnlocked(const std::string& targetMap) {
    if (unlockedPortalCount < 50) {
        unlockedPortals[unlockedPortalCount] = targetMap;
        unlockedPortalCount++;
    }
}

Gate* GameMap::CheckGateInteraction(Rectangle playerBounds) {
    // Expand the player's reach by 10 pixels in all directions
    Rectangle reach = {
        playerBounds.x - 10,
        playerBounds.y - 10,
        playerBounds.width  + 20,
        playerBounds.height + 20
    };
    
    for (int i = 0; i < gateCount; i++) {
        // Only allow interacting with locked gates
        if (gates[i].isLocked && CheckCollisionRecs(reach, gates[i].bounds)) {
            return &gates[i];
        }
    }
    return nullptr;
}

void GameMap::MarkGateUnlocked(Gate* gate) {
    gate->isLocked = false;
    
    // Save it to history so it doesn't respawn when you leave and come back
    if (unlockedGateCount < 100) {
        unlockedGates[unlockedGateCount++] = gate->uniqueID;
    }
}