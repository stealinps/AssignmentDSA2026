#include "map.h"
#include <fstream>
#include <iostream>

GameMap::GameMap() {
    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLS; x++) {
            grid[y][x] = 0;
        }
    }

    wallSprite = LoadTexture("src/sprite/wall.png"); 
    portalSprite = LoadTexture("src/sprite/door.png");

    //Initialize our manual counter to 0
    portalCount = 0; 
}

GameMap::~GameMap() {
    UnloadTexture(wallSprite); 
    UnloadTexture(portalSprite);
}

bool GameMap::LoadMap(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "ERROR: Could not load map file: " << filename << std::endl;
        return false;
    }

    // Setting the counter to 0 makes the engine overwrite old portals automatically
    portalCount = 0; 

    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLS; x++) {
            file >> grid[y][x]; 
        }
    }

    std::string marker;
    if (file >> marker && marker == "PORTALS") {
        int numPortals;
        file >> numPortals; 
        
        for (int i = 0; i < numPortals; i++) {
            int gridX, gridY, spawnGridX, spawnGridY;
            std::string target;
            
            file >> gridX >> gridY >> target >> spawnGridX >> spawnGridY;
            
            float pixelX = gridX * TILE_SIZE;
            float pixelY = gridY * TILE_SIZE;
            float spawnPixelX = spawnGridX * TILE_SIZE;
            float spawnPixelY = spawnGridY * TILE_SIZE;
            
            AddPortal({pixelX, pixelY, TILE_SIZE, TILE_SIZE}, target, spawnPixelX, spawnPixelY);
        }
    }

    file.close();
    return true;
}

void GameMap::AddPortal(Rectangle bounds, std::string targetMap, float spawnX, float spawnY) {
    // Ensure we don't exceed hardcoded array limit
    if (portalCount < MAX_PORTALS) {
        Portal newPortal;
        newPortal.bounds = bounds;
        newPortal.targetMap = targetMap;
        newPortal.spawnX = spawnX;
        newPortal.spawnY = spawnY;

        // Slot the new portal into the array at the current count, then increment the count
        portals[portalCount] = newPortal;
        portalCount++;
    } else {
        std::cout << "WARNING: Max portals reached. Cannot add more!" << std::endl;
    }
}

bool GameMap::CheckPortals(Rectangle playerBounds, Portal& outPortal) {
    // REPLACED: portals.size() with our manual portalCount
    for (int i = 0; i < portalCount; i++) {
        if (CheckCollisionRecs(playerBounds, portals[i].bounds)) {
            outPortal = portals[i];
            return true; 
        }
    }
    return false; 
}

void GameMap::Draw() {
    // Draw the walls
    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLS; x++) {
            if (grid[y][x] == 1) {
                DrawTexture(wallSprite, x * TILE_SIZE, y * TILE_SIZE, WHITE);
            }
        }
    }

    // Draw the portals
    for (int i = 0; i < portalCount; i++) {
        // Cast the float coordinates back to integers (int) for pixel-perfect drawing
        int drawX = (int)portals[i].bounds.x;
        int drawY = (int)portals[i].bounds.y;
        
        // Stamp the portal sprite onto the screen!
        DrawTexture(portalSprite, drawX, drawY, WHITE);
    }
}

bool GameMap::IsSolid(int targetX, int targetY) {
    // BOUNDS CHECK: Prevent crashing if checking outside the screen
    if (targetX < 0 || targetX >= MAP_COLS || targetY < 0 || targetY >= MAP_ROWS) {
        return true; 
    }

    // TILE CHECK: Is the destination a wall
    if (grid[targetY][targetX] == 1) {
        return true;
    }

    return false; 
}

bool GameMap::CheckCollision(Rectangle rect) {
    // Convert the pixel boundaries of the bounding box into grid rows and columns
    int startCol = (int)(rect.x / TILE_SIZE);
    int endCol = (int)((rect.x + rect.width) / TILE_SIZE);
    int startRow = (int)(rect.y / TILE_SIZE);
    int endRow = (int)((rect.y + rect.height) / TILE_SIZE);

    // Scan only the specific subset of tiles the bounding box is currently touching
    for (int row = startRow; row <= endRow; row++) {
        for (int col = startCol; col <= endCol; col++) {
            if (IsSolid(col, row)) {
                return true; // Hit a wall!
            }
        }
    }
    return false; // Safe to walk
}