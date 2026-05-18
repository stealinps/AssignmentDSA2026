#include "raylib.h"
#include "config.h"
#include "player.h"
#include "renderer.h"
#include "map.h"
#include "dialogue.h"

int main() {
    Renderer gameRenderer;   
    
    // Setup the starting world
    GameMap worldMap;        
    worldMap.LoadMap("src/levels/level1.txt"); 
    
    Player myPlayer(388.0f, 256.0f); // Starting Point
    GameState currentState = STATE_OVERWORLD;

    DialogueBox dialogueBox;

    while (!WindowShouldClose()) {
        switch (currentState) {
            case STATE_OVERWORLD: {
                myPlayer.Update(worldMap); 

                // CHEST INTERACTION
                // If the player presses 'E', check if a chest is nearby
                if (IsKeyPressed(KEY_E)) {
                    Chest* nearbyChest = worldMap.CheckChestInteraction(myPlayer.GetBounds());
            
                    // Safety Check: Did we actually find a chest, AND is it closed?
                        if (nearbyChest != nullptr && !nearbyChest->isOpen) {
                
                            // Attempt to push the chest's item into the player's array
                            if (myPlayer.AddItem(nearbyChest->content)) {
                                worldMap.MarkChestOpened(nearbyChest); 
                                std::cout << "[PLAYER] Picked up item! Chest opened." << std::endl;
                    
                                // Success! Modify the chest directly in memory to mark it empty
                                nearbyChest->isOpen = true; 
                            } else {
                                std::cout << "[ERROR] Inventory full! Chest stays closed." << std::endl;
                            }
                        }
                }

                // --- SIGNPOST LOGIC ---
                if (IsKeyPressed(KEY_R)) {
                    
                    // Ask the map if we are standing near a sign
                    Signpost* nearbySign = worldMap.CheckSignpostInteraction(myPlayer.GetBounds());
                    
                    if (nearbySign != nullptr) {
                        // 1. Wipe out any old dialogue
                        dialogueBox.Start(); 
                        
                        // 2. Loop through the specific sign's memory and push its text!
                        for (int i = 0; i < nearbySign->lineCount; i++) {
                            dialogueBox.Enqueue(nearbySign->dialogue[i]);
                        }
                        
                        // 3. Freeze the game
                        currentState = STATE_DIALOGUE;
                    }
                }

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
            case STATE_DIALOGUE: {
                
                // NEW: Make sure the dialogue box ticks forward every frame
                dialogueBox.Update(); 

                if (IsKeyPressed(KEY_SPACE)) {
                    
                    // Did the typewriter finish?
                    if (dialogueBox.IsTextFinished()) {
                        dialogueBox.Dequeue(); // Move to next sentence
                        
                        if (!dialogueBox.IsActive()) {
                            currentState = STATE_OVERWORLD; // Close the box
                        }
                    } 
                    // If it's STILL typing, pressing space instantly finishes it
                    else {
                        dialogueBox.SkipTyping(); 
                    }
                }
                break;
            }
            

            case STATE_MENU: // No update function called so the game freezes
                if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) currentState = STATE_OVERWORLD;
                break;
                
            case STATE_BATTLE:
                if (IsKeyPressed(KEY_ESCAPE)) currentState = STATE_OVERWORLD;
                break;
            
        }

        BeginDrawing();
        //ClearBackground(BLACK);
        
        // 1. Let your custom Renderer draw the map, the player, the menu, or the battle!
        gameRenderer.DrawFrame(currentState, myPlayer, worldMap);

        // 2. Draw the Dialogue UI on the very top of whatever the renderer just painted.
        // (dialogueBox.Draw() already checks if it is active, so it's safe to just call it here)
        dialogueBox.Draw(); 

        EndDrawing();
    }
    return 0;
}