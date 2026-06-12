#include "map.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <ctime>
 
GameMap::GameMap() {
    for (int y = 0; y < MAP_ROWS; y++)
        for (int x = 0; x < MAP_COLS; x++)
            grid[y][x] = 0;
 
    wallSprite        = LoadTexture("src/sprite/wall.png");
    portalSprite      = LoadTexture("src/sprite/door.png");
    portalLockedSprite= LoadTexture("src/sprite/door_locked.png");
    chestClosedSprite = LoadTexture("src/sprite/chest_closed.png");
    chestOpenSprite   = LoadTexture("src/sprite/chest_open.png");
    signSprite        = LoadTexture("src/sprite/sign.png");
    enemySprite       = LoadTexture("src/sprite/enemy.png");
 
    portalCount   = 0;
    chestCount    = 0;
    historyCount  = 0;
    signpostCount = 0;
    enemyCount    = 0;
    defeatedCount = 0;
    isAggro       = false;
 
    std::srand(std::time(nullptr));
}
 
GameMap::~GameMap() {
    UnloadTexture(wallSprite);
    UnloadTexture(portalSprite);
    UnloadTexture(chestClosedSprite);
    UnloadTexture(chestOpenSprite);
    UnloadTexture(signSprite);
    UnloadTexture(enemySprite);
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
            item.description = "Opens locked portals and doors";
            break;
        default:
            item.description = "A mysterious item";
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
 
    // Read grid
    for (int y = 0; y < MAP_ROWS; y++)
        for (int x = 0; x < MAP_COLS; x++)
            file >> grid[y][x];
 
    std::string marker;
    while (file >> marker) {
 
        // ---- PORTALS ----
        if (marker == "PORTALS") {
            int numPortals;
            file >> numPortals;
            for (int i = 0; i < numPortals; i++) {
                int gridX, gridY, spawnGridX, spawnGridY;
                std::string target;
                file >> gridX >> gridY >> target >> spawnGridX >> spawnGridY;
 
                // Optional requiresKey field (0 or 1).
                // Backward-compatible: if the next token is not "0"/"1"
                // (e.g. another section marker), seek back and default to false.
                bool requiresKey = false;
                std::streampos savedPos = file.tellg();
                std::string peekTok;
                if (file >> peekTok) {
                    if (peekTok == "0" || peekTok == "1") {
                        requiresKey = (peekTok == "1");
                    } else {
                        file.seekg(savedPos); // Put the token back
                    }
                }
 
                float px = (float)(gridX      * TILE_SIZE);
                float py = (float)(gridY      * TILE_SIZE);
                float sx = (float)(spawnGridX * TILE_SIZE);
                float sy = (float)(spawnGridY * TILE_SIZE);
 
                AddPortal({px, py, (float)TILE_SIZE, (float)TILE_SIZE},
                          target, sx, sy, requiresKey);
            }
        }
 
        // ---- CHESTS ----
        else if (marker == "CHESTS") {
            int numChests;
            file >> numChests;
            for (int i = 0; i < numChests; i++) {
                int gridX, gridY, chestID, itemID;
                std::string itemName;
                file >> gridX >> gridY >> chestID >> itemID >> itemName;
 
                float px = (float)(gridX * TILE_SIZE);
                float py = (float)(gridY * TILE_SIZE);
 
                Chest newChest;
                newChest.bounds   = {px, py, (float)TILE_SIZE, (float)TILE_SIZE};
                newChest.uniqueID = chestID;
                newChest.isOpen   = false;
 
                // BUG FIX: Set quantity=1 and fill description.
                // The old code did {itemID, itemName} leaving quantity=0,
                // which made UseItem return false (0 >= 1 is false).
                newChest.content = {itemID, itemName, "", 1};
                FillItemData(newChest.content);
 
                std::cout << "[LOADER] Chest ID " << chestID
                          << " | item: " << itemName
                          << " | qty: " << newChest.content.quantity << std::endl;
 
                // Restore open state from history
                for (int j = 0; j < historyCount; j++) {
                    if (openedHistory[j] == chestID) {
                        newChest.isOpen       = true;
                        newChest.content.id   = 0;
                        std::cout << "[LOADER] Chest " << chestID << " already opened." << std::endl;
                        break;
                    }
                }
 
                if (chestCount < MAX_CHESTS) {
                    chests[chestCount] = newChest;
                    chestCount++;
                }
            }
        }
 
        // ---- SIGNPOSTS ----
        else if (marker == "SIGNPOSTS") {
            int numSigns;
            file >> numSigns;
            for (int i = 0; i < numSigns; i++) {
                int gridX, gridY, numLines;
                file >> gridX >> gridY >> numLines;
 
                Signpost newSign;
                newSign.bounds    = {(float)(gridX * TILE_SIZE),
                                     (float)(gridY * TILE_SIZE),
                                     (float)TILE_SIZE, (float)TILE_SIZE};
                newSign.lineCount = 0;
 
                std::string dummy;
                std::getline(file, dummy); // Eat trailing newline after numLines
 
                for (int j = 0; j < numLines; j++) {
                    std::string line;
                    std::getline(file, line);
                    if (newSign.lineCount < MAX_LINES_PER_SIGNPOST) {
                        newSign.dialogue[newSign.lineCount] = line;
                        newSign.lineCount++;
                    }
                }
 
                if (signpostCount < MAX_SIGNPOSTS) {
                    signposts[signpostCount] = newSign;
                    signpostCount++;
                }
            }
        }
 
        // ---- ENEMIES ----
        else if (marker == "ENEMIES") {
            int numEnemies;
            file >> numEnemies;
            for (int i = 0; i < numEnemies; i++) {
                int   gridX, gridY, enemyID, aggroRange;
                float speed;
                file >> gridX >> gridY >> enemyID >> aggroRange >> speed;
 
                float px = (float)(gridX * TILE_SIZE);
                float py = (float)(gridY * TILE_SIZE);
 
                const float hitW = 20.0f;
                const float hitH = 20.0f;
                float offsetX = (TILE_SIZE - hitW) / 2.0f;
                float offsetY =  TILE_SIZE - hitH;
 
                Enemy newEnemy;
                newEnemy.bounds   = {px + offsetX, py + offsetY, hitW, hitH};
                newEnemy.uniqueID = enemyID;
                newEnemy.aggroRange = aggroRange;
                newEnemy.speed    = speed;
                newEnemy.spawnX   = px + offsetX;
                newEnemy.spawnY   = py + offsetY;
                newEnemy.isDefeated = false;
                newEnemy.hasLoot  = true;
 
                // Random loot table: 50% str potion, 30% def potion, 20% iron key
                int roll = rand() % 100;
                if (roll < 50) {
                    newEnemy.lootDrop = {ITEM_STRENGTH_POTION, "Strength Potion",
                                         "Deal +10 damage for 2 turns", 1};
                } else if (roll < 80) {
                    newEnemy.lootDrop = {ITEM_DEFENSE_POTION, "Defense Potion",
                                         "Gain +50 max HP", 1};
                } else {
                    newEnemy.lootDrop = {ITEM_IRON_KEY, "Iron Key",
                                         "Opens locked portals and doors", 1};
                }
 
                // Restore defeated state from history
                for (int j = 0; j < defeatedCount; j++) {
                    if (defeatedHistory[j] == enemyID) {
                        newEnemy.isDefeated = true;
                        newEnemy.hasLoot    = false;
                        std::cout << "[LOADER] Enemy " << enemyID << " already defeated." << std::endl;
                        break;
                    }
                }
 
                if (enemyCount < MAX_ENEMIES) {
                    enemies[enemyCount] = newEnemy;
                    enemyCount++;
                }
            }
        }
    }
 
    file.close();
    return true;
}
 
// -------------------------------------------------------------------
// PORTALS
// -------------------------------------------------------------------
void GameMap::AddPortal(Rectangle bounds, std::string targetMap,
                        float spawnX, float spawnY, bool requiresKey) {
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
    portals[portalCount++] = p;
}
 
bool GameMap::CheckPortals(Rectangle playerBounds, Portal& outPortal) {
    for (int i = 0; i < portalCount; i++) {
        if (CheckCollisionRecs(playerBounds, portals[i].bounds)) {
            outPortal = portals[i];
            return true;
        }
    }
    return false;
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
        // Color tint = portals[i].requiresKey ? RED : WHITE;
        // Change texture depending if the door is locked or not
        if (portals[i].requiresKey) {
            DrawTexture(portalSprite, drawX, drawY, WHITE);
        } else {
            DrawTexture(portalLockedSprite, drawX, drawY, WHITE);
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
        DrawTexture(enemySprite, drawX, drawY, isAggro ? RED : WHITE);
        DrawRectangleLinesEx(enemies[i].bounds, 1, RED); // Debug hitbox
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
    int startCol = (int)(rect.x / TILE_SIZE);
    int endCol   = (int)((rect.x + rect.width)  / TILE_SIZE);
    int startRow = (int)(rect.y / TILE_SIZE);
    int endRow   = (int)((rect.y + rect.height) / TILE_SIZE);
    for (int row = startRow; row <= endRow; row++)
        for (int col = startCol; col <= endCol; col++)
            if (IsSolid(col, row)) return true;
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
