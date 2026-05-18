#include "raylib.h"
#include "config.h"
#include "player.h"
#include "renderer.h"
#include "map.h"

int main() {
    Renderer gameRenderer;   
    
    // Setup the starting world
    GameMap worldMap;        
    worldMap.LoadMap("src/levels/level1.txt"); 
    
    Player myPlayer(400.0f, 300.0f); // Starting Point
    GameState currentState = STATE_OVERWORLD;

    while (!WindowShouldClose()) {
        switch (currentState) {
            case STATE_OVERWORLD: {
                myPlayer.Update(worldMap); 
                
                // PORTAL CHECKING LOGIC
                Portal hitPortal;
                if (worldMap.CheckPortals(myPlayer.GetBounds(), hitPortal)) {
                    
                    // Load the new map (The file itself will automatically spawn the new doors)
                    worldMap.LoadMap(hitPortal.targetMap);
                    
                    // Teleport the player
                    myPlayer.Teleport(hitPortal.spawnX, hitPortal.spawnY);
                }
                
                if (IsKeyPressed(KEY_B)) currentState = STATE_BATTLE;
                if (IsKeyPressed(KEY_M)) currentState = STATE_MENU; 
                break;
            }

            case STATE_MENU: // No update function called so the game freezes
                if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) currentState = STATE_OVERWORLD;
                break;
                
            case STATE_BATTLE:
                if (IsKeyPressed(KEY_ESCAPE)) currentState = STATE_OVERWORLD;
                break;
            
        }

        gameRenderer.DrawFrame(currentState, myPlayer, worldMap);
    }
    return 0;
}