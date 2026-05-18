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
    chestClosedSprite = LoadTexture("src/sprite/chest_closed.png");
    chestOpenSprite = LoadTexture("src/sprite/chest_open.png");
    signSprite = LoadTexture("src/sprite/sign.png");

    //Initialize our manual counter to 0
    portalCount = 0; 
    chestCount = 0; 
    historyCount = 0;
    signpostCount = 0;
}

GameMap::~GameMap() {
    UnloadTexture(wallSprite); 
    UnloadTexture(portalSprite);
    UnloadTexture(chestClosedSprite);
    UnloadTexture(chestOpenSprite);
    UnloadTexture(signSprite);
}

bool GameMap::LoadMap(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "ERROR: Could not load map file: " << filename << std::endl;
        return false;
    }

    // 1. CLEAR OLD DATA! 
    portalCount = 0; 
    chestCount = 0; // NEW: Wipe out the old chests so they don't duplicate

    // 2. Read the Map Grid
    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLS; x++) {
            file >> grid[y][x]; 
        }
    }

    // 3. THE ARCHITECTURE UPGRADE: Read all remaining chunks of data
    std::string marker;
    
    // This loop runs continuously, sucking up one word at a time, until the file ends.
    while (file >> marker) {
        
        if (marker == "PORTALS") {
            int numPortals;
            file >> numPortals; 
            for (int i = 0; i < numPortals; i++) {
                int gridX, gridY, spawnGridX, spawnGridY;
                std::string target;
                file >> gridX >> gridY >> target >> spawnGridX >> spawnGridY;
                
                float px = (float)(gridX * TILE_SIZE);
                float py = (float)(gridY * TILE_SIZE);
                float sx = (float)(spawnGridX * TILE_SIZE);
                float sy = (float)(spawnGridY * TILE_SIZE);
                
                AddPortal({px, py, (float)TILE_SIZE, (float)TILE_SIZE}, target, sx, sy);
            }
        } 

        else if (marker == "CHESTS") { 
            int numChests;
            file >> numChests;
            for (int i = 0; i < numChests; i++) {
                int gridX, gridY, chestID, itemID;
                std::string itemName;
                
                // Read the new ChestID from the file
                file >> gridX >> gridY >> chestID >> itemID >> itemName;

                // Inside LoadMap(), parsing the CHESTS...
                float px = (float)(gridX * TILE_SIZE);
                float py = (float)(gridY * TILE_SIZE);
                
                Chest newChest;
                // Use our new float variables and cast TILE_SIZE!
                
                newChest.bounds = {px, py, (float)TILE_SIZE, (float)TILE_SIZE};
                //newChest.bounds = {gridX * TILE_SIZE, gridY * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                newChest.content = {itemID, itemName};
                newChest.isOpen = false;
                newChest.uniqueID = chestID; // Store the ID!

                std::cout << "[LOADER] Reading blueprint for Chest ID: " << chestID << std::endl;

                // --- THE NEW, ULTRA-FAST MEMORY CHECK ---
                for (int j = 0; j < historyCount; j++) {
                    // Just compare the two numbers!
                    if (openedHistory[j] == chestID) {
                        newChest.isOpen = true; 
                        newChest.content.id = 0; 
                        std::cout << "[LOADER] Match found in history! Forcing ID " << chestID << " open." << std::endl;
                        break; 
                    }
                }
                // ----------------------------------------

                if (chestCount < MAX_CHESTS) {
                    chests[chestCount] = newChest;
                    chestCount++;
                }
            }
        }
        
        else if (marker == "SIGNPOSTS") {
            int numSigns;
            file >> numSigns;
            
            for (int i = 0; i < numSigns; i++) {
                int gridX, gridY, numLines;
                file >> gridX >> gridY >> numLines;

                Signpost newSign;
                newSign.bounds = {(float)(gridX * TILE_SIZE), (float)(gridY * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE};
                newSign.lineCount = 0;

                // THE C++ TRAP FIX: 
                // We must "eat" the invisible Enter key (\n) left behind by file >>
                std::string dummy;
                std::getline(file, dummy); 

                // Now we read the actual sentences
                for (int j = 0; j < numLines; j++) {
                    std::string line;
                    std::getline(file, line); // Grabs the whole line, spaces included!
                    
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

    // Draw the chest
    for (int i = 0; i < chestCount; i++) {
        int drawX = (int)chests[i].bounds.x;
        int drawY = (int)chests[i].bounds.y;
        if (chests[i].isOpen) {
            // If it's been looted, draw the empty open chest
            DrawTexture(chestOpenSprite, drawX, drawY, WHITE);
        } else {
            // If it hasn't been looted, draw the shiny closed chest
            DrawTexture(chestClosedSprite, drawX, drawY, WHITE);
        }
    }

    // Draw the signpost
    for (int i = 0; i < signpostCount; i++) {
        
        // Grab the exact X and Y pixel coordinates, casting them to integers for the screen
        int drawX = (int)signposts[i].bounds.x;
        int drawY = (int)signposts[i].bounds.y;

        // Stamp the signpost onto the map
        DrawTexture(signSprite, drawX, drawY, WHITE);
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

void GameMap::AddChest(Rectangle bounds, Item content) {
    if (chestCount < MAX_CHESTS) {
        Chest newChest;
        newChest.bounds = bounds;
        newChest.content = content;
        newChest.isOpen = false;
        
        chests[chestCount] = newChest;
        chestCount++;
    }
}

// THE INTERACTION CHECK
Chest* GameMap::CheckChestInteraction(Rectangle playerBounds) {
    // We create a "reach" box that is slightly larger than the player
    // so they don't have to be perfectly colliding with the chest to open it.
    Rectangle reach = { 
        playerBounds.x - 10, 
        playerBounds.y - 10, 
        playerBounds.width + 20, 
        playerBounds.height + 20 
    };

    for (int i = 0; i < chestCount; i++) {
        if (CheckCollisionRecs(reach, chests[i].bounds)) {
            // We found a chest! Return its exact memory address using the '&' symbol
            return &chests[i]; 
        }
    }
    
    return nullptr; // nullptr means "We found absolutely nothing."
}

void GameMap::MarkChestOpened(Chest* chest) {
    chest->isOpen = true;
    chest->content.id = 0; 
    
    if (historyCount < MAX_HISTORY) {
        // Just write down the ID number in our journal!
        openedHistory[historyCount] = chest->uniqueID;
        historyCount++; 

        std::cout << "[MEMORY] Saved Chest ID: " << chest->uniqueID 
                  << " | Total opened: " << historyCount << std::endl;
    }
}

Signpost* GameMap::CheckSignpostInteraction(Rectangle playerBounds) {
    // Create the "reach" box
    Rectangle reach = { 
        playerBounds.x - 10, playerBounds.y - 10, 
        playerBounds.width + 20, playerBounds.height + 20 
    };

    for (int i = 0; i < signpostCount; i++) {
        if (CheckCollisionRecs(reach, signposts[i].bounds)) {
            return &signposts[i]; 
        }
    }
    return nullptr;
}