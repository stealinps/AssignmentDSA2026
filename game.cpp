#include "game.h"
#include "config.h"
#include <fstream>
#include <raylib.h>

// --- INITIALIZATION ---
Game::Game() : myPlayer(0.0f, 0.0f) {
    worldMap.LoadMap("src/levels/spawn.txt");

    myPlayer.Teleport(worldMap.defaultSpawnX, worldMap.defaultSpawnY);

    currentState = STATE_MAIN_MENU;
    mainMenuSelection = 0;
    playerNameInput = "";
    leaderboardCount = 0;
    playTimer = 0.0f;
    gameBeat = false;
    currentEnemy = nullptr;
    fileScore = 0;

    // FIX: Prevent portal spam loop
    wasTouchingPortal = false;
}

// --- CLEANUP ---
Game::~Game() {
}

// --- MAIN LOOP ---
void Game::Run() {
    while (!WindowShouldClose()) {
        ProcessInput();
        Update();
        Draw();
    }
}

// --- INPUT HANDLING ---
void Game::ProcessInput() {
    playerInput = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        playerInput.y -= 1.0f;

    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
        playerInput.y += 1.0f;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        playerInput.x -= 1.0f;

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        playerInput.x += 1.0f;
}

// --- GAME LOGIC ---
void Game::Update() {
    // Only tick the timer if we are actually playing the game
    if (currentState == STATE_OVERWORLD || currentState == STATE_BATTLE) {
        playTimer += GetFrameTime();
    }

    switch (currentState) {
        // ============================================================
        case STATE_MAIN_MENU: {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                mainMenuSelection--;
                if (mainMenuSelection < 0) mainMenuSelection = 1;
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                mainMenuSelection++;
                if (mainMenuSelection > 1) mainMenuSelection = 0;
            }
            if (IsKeyPressed(KEY_ENTER)) {
                if (mainMenuSelection == 0) {
                    currentState = STATE_NAME_INPUT;
                    playerNameInput = ""; // Reset name field
                } else {
                    LoadLeaderboard(); // Load fresh data before showing
                    currentState = STATE_LEADERBOARD;
                }
            }
            break;
        }

        // ============================================================
        case STATE_NAME_INPUT: {
            // Raylib function to grab pressed characters (for typing)
            int key = GetCharPressed();

            // Check if more characters have been pressed on the same frame
            while (key > 0) {
                // Only allow standard typing characters and limit length
                if ((key >= 32) && (key <= 125) && (playerNameInput.length() < 12)) {
                    playerNameInput += (char)key;
                }
                key = GetCharPressed(); // Check next key in queue
            }

            // Handle backspace
            if (IsKeyPressed(KEY_BACKSPACE) && playerNameInput.length() > 0) {
                playerNameInput.pop_back(); 
            }

            // Confirm Name
            if (IsKeyPressed(KEY_ENTER) && playerNameInput.length() > 0) {
                myPlayer.SetName(playerNameInput);

                // FULL GAME RESET
                worldMap.ResetProgress();                                                      // 1. Erase chest/enemy history
                worldMap.LoadMap("src/levels/spawn.txt");                             // 2. Load the map first to grab SPAWN point                                                                               
                myPlayer.Reset(worldMap.defaultSpawnX, worldMap.defaultSpawnY); // 3. Reset player stats using the new map's spawn coordinates
                battle.ResetPlayerStats();                                                     // 4. Reset battle HP/Attack
                fileScore = 0;
                playTimer = 0.0f;
                currentState = STATE_OVERWORLD;
            } 
            
            // Go back
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentState = STATE_MAIN_MENU;
            }
            break;
        }

        // ============================================================
        case STATE_LEADERBOARD: {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
                currentState = STATE_MAIN_MENU;
            }
            break;
        }

        // ============================================================
        case STATE_OVERWORLD: {

            myPlayer.Update(worldMap, playerInput);

            worldMap.UpdateEnemies(myPlayer.GetBounds());

            // --------------------------------------------------------
            // ENEMY COLLISION -> START BATTLE
            // --------------------------------------------------------
            Enemy* touchedEnemy =
                worldMap.CheckEnemyCollision(myPlayer.GetBounds());

            if (touchedEnemy != nullptr) {
                currentEnemy = touchedEnemy;

                if (touchedEnemy->typeID == 999) {
                    battle.StartBossBattle(
                        touchedEnemy->name, 
                        touchedEnemy->maxHp, 
                        touchedEnemy->attack, 
                        touchedEnemy->expReward, 
                        touchedEnemy->scoreReward
                    );
                } else {
                    battle.StartBattle(
                        touchedEnemy->name, 
                        touchedEnemy->maxHp, 
                        touchedEnemy->attack, 
                        touchedEnemy->expReward, 
                        touchedEnemy->scoreReward
                    );
                }

                currentState = STATE_BATTLE;
                break;
            }

            // --------------------------------------------------------
            // CHEST/GATE INTERACTION (E)
            // --------------------------------------------------------
            if (IsKeyPressed(KEY_E)) {

                // Chests
                Chest* nearbyChest =
                    worldMap.CheckChestInteraction(myPlayer.GetBounds());

                if (nearbyChest != nullptr &&
                    !nearbyChest->isOpen) {

                    if (myPlayer.AddItem(nearbyChest->content)) {

                        worldMap.MarkChestOpened(nearbyChest);

                        nearbyChest->isOpen = true;

                        fileScore += 10;

                        dialogueBox.Start();

                        dialogueBox.Enqueue(
                            "You found: " +
                            nearbyChest->content.name + "!"
                        );

                        currentState = STATE_DIALOGUE;

                    } else {

                        dialogueBox.Start();

                        dialogueBox.Enqueue(
                            "Your inventory is full!"
                        );

                        currentState = STATE_DIALOGUE;
                    }
                }

                // Gates
                Gate* nearbyGate = worldMap.CheckGateInteraction(myPlayer.GetBounds());

                if (nearbyGate != nullptr && nearbyGate->isLocked) {
                    
                    // Check if the player has at least 1 of the specific required item
                    if (myPlayer.GetItemQuantity(nearbyGate->requiredItemID) > 0) {
                        
                        // Consume the specific item
                        myPlayer.UseItem(nearbyGate->requiredItemID);
                        worldMap.MarkGateUnlocked(nearbyGate);
                        
                        dialogueBox.Start();
                        dialogueBox.Enqueue("You unlocked the gate.");
                        currentState = STATE_DIALOGUE;
                    } else {
                        dialogueBox.Start();
                        dialogueBox.Enqueue("The gate is locked.");
                        currentState = STATE_DIALOGUE;
                    }
                }
            }

            // --------------------------------------------------------
            // SIGNPOST INTERACTION (R)
            // --------------------------------------------------------
            if (IsKeyPressed(KEY_R)) {

                Signpost* nearbySign =
                    worldMap.CheckSignpostInteraction(
                        myPlayer.GetBounds()
                    );

                if (nearbySign != nullptr) {

                    dialogueBox.Start();

                    for (int i = 0;
                         i < nearbySign->lineCount;
                         i++) {

                        dialogueBox.Enqueue(
                            nearbySign->dialogue[i]
                        );
                    }

                    currentState = STATE_DIALOGUE;
                }
            }

            // --------------------------------------------------------
            // PORTAL SYSTEM
            // FIXED: Prevent infinite dialogue spam
            // --------------------------------------------------------
            {
                // Pointer method used
                Portal* hitPortal = worldMap.CheckPortals(myPlayer.GetBounds());
                bool touchingPortal = (hitPortal != nullptr);

                // Trigger only when entering portal
                if (touchingPortal && !wasTouchingPortal) {

                    // ---------------- LOCKED PORTAL ----------------
                    if (hitPortal->requiresKey) {
                        
                        if (myPlayer.GetItemQuantity(hitPortal->requiredItemID) == 0) {
                            dialogueBox.Start();
                            dialogueBox.Enqueue("The door is locked.");
                            currentState = STATE_DIALOGUE;
                        } 
                        else {
                            // Unlock the door permanently
                            myPlayer.UseItem(hitPortal->requiredItemID);
                            hitPortal->requiresKey = false;

                            worldMap.MarkPortalUnlocked(hitPortal->targetMap);
                            
                            dialogueBox.Start();
                            dialogueBox.Enqueue("You unlocked the door.");
                            currentState = STATE_DIALOGUE;
                        }
                    }

                    // ---------------- NORMAL PORTAL ----------------
                    if (!hitPortal->requiresKey) {
                        // SAVE THE DATA BEFORE OVERWRITING THE MAP
                        std::string nextMap = hitPortal->targetMap;
                        float destX = hitPortal->spawnX;
                        float destY = hitPortal->spawnY;

                        // Load next map safely
                        worldMap.LoadMap(nextMap);

                        // Teleport player safely
                        myPlayer.Teleport(destX, destY);

                        // Safety collision check
                        const float offX[] = { 50.0f, -50.0f, 0.0f, 0.0f };
                        const float offY[] = { 0.0f, 0.0f, -50.0f, 50.0f };

                        for (int i = 0; i < 4; i++) {
                            if (worldMap.CheckEnemyCollision(myPlayer.GetBounds()) == nullptr) {
                                break;
                            }
                            myPlayer.Teleport(destX + offX[i], destY + offY[i]);
                        }
                    }
                }

                // Update after processing
                wasTouchingPortal = touchingPortal;
            }

            // --------------------------------------------------------
            // MENU
            // --------------------------------------------------------
            if (IsKeyPressed(KEY_M)) {

                currentState = STATE_MENU;
            }

            // --------------------------------------------------------
            // DEBUG BOSS FIGHT (press B to jump straight to the boss)
            // --------------------------------------------------------
            if (IsKeyPressed(KEY_B)) {

                static Enemy debugBoss;
                debugBoss.typeID = 999;
                debugBoss.name = "THE COMPILER";
                debugBoss.maxHp = 150;
                debugBoss.attack = 25;
                debugBoss.expReward = 0;
                debugBoss.scoreReward = 0;

                // Point the game's tracking pointer to our fake boss
                currentEnemy = &debugBoss;

                // Start the battle using the dummy's stats
                battle.StartBossBattle(debugBoss.name, debugBoss.maxHp, debugBoss.attack, debugBoss.expReward, debugBoss.scoreReward);

                currentState = STATE_BATTLE;
            }

            // --------------------------------------------------------
            // DEBUG NORMAL FIGHT (press N for a standard enemy battle)
            // --------------------------------------------------------
            if (IsKeyPressed(KEY_N)) {

                battle.StartBattle("Dummy", 20, 1, 0, 0);

                currentEnemy = nullptr;

                currentState = STATE_BATTLE;
            }

            // --------------------------------------------------------
            // DEBUG VICTORY SCREEN(press V for a insta win)
            // --------------------------------------------------------
            if (IsKeyPressed(KEY_V)) {
                fileScore += 9999; // Fake Score
                
                currentState = STATE_VICTORY;
            }

            break;
        }

        // ============================================================
        case STATE_DIALOGUE: {

            dialogueBox.Update();

            if (IsKeyPressed(KEY_SPACE)) {

                if (dialogueBox.IsTextFinished()) {

                    dialogueBox.Dequeue();

                    if (!dialogueBox.IsActive()) {
                        // Check if game is beaten
                        if (gameBeat) {
                            currentState = STATE_VICTORY;
                            gameBeat = false; // Reset for the next playthrough
                        } else {
                            currentState = STATE_OVERWORLD;
                        }
                    }

                } else {

                    dialogueBox.SkipTyping();
                }
            }

            break;
        }

        // ============================================================
        case STATE_MENU: {

            if (IsKeyPressed(KEY_M) ||
                IsKeyPressed(KEY_ESCAPE)) {

                currentState = STATE_OVERWORLD;
            }

            break;
        }

        // ============================================================
        case STATE_BATTLE: {

            if (!battle.IsBattleOver()) {

                battle.Update(myPlayer);
            }

            if (battle.IsBattleOver()) {

                if (IsKeyPressed(KEY_SPACE) ||
                    IsKeyPressed(KEY_ESCAPE)) {

                    // ------------------------------------------------
                    // PLAYER LOST
                    // ------------------------------------------------
                    if (battle.GetState() == PLAYER_LOSE) {

                        worldMap.ResetDefeatedEnemies();

                        worldMap.LoadMap(
                            "src/levels/spawn.txt"
                        );

                        myPlayer.Teleport(
                            388.0f,
                            256.0f
                        );

                        battle.get_healing() =
                            battle.max_HP();

                        currentEnemy = nullptr;

                        battle.StartBattle("Reset", 1, 1, 0, 0);

                        currentState = STATE_OVERWORLD;
                    }

                    // ------------------------------------------------
                    // PLAYER WON
                    // ------------------------------------------------
                    else {

                        std::string lootMsg = "";

                        if (currentEnemy != nullptr) {
                            // Check if this was the Final Boss
                            if (currentEnemy->typeID == 999) {
                                fileScore += 1000; // Big bonus for winning
                                SaveToLeaderboard();
                                gameBeat = true;
                                lootMsg = "You defeated the Final Boss! Score saved.";
                            }

                            Item loot =
                                worldMap.GetEnemyLoot(
                                    currentEnemy
                                );

                            worldMap.MarkEnemyDefeated(
                                currentEnemy
                            );

                            currentEnemy = nullptr;

                            if (loot.id != 0) {

                                if (myPlayer.AddItem(loot)) {

                                    lootMsg =
                                        "You received: " +
                                        loot.name + "!";

                                    fileScore += 20;

                                } else {

                                    lootMsg =
                                        "Inventory full! " +
                                        loot.name +
                                        " was lost.";
                                }
                            }
                        }

                        battle.StartBattle("Reset", 1, 1, 0, 0);

                        if (!lootMsg.empty()) {

                            dialogueBox.Start();

                            dialogueBox.Enqueue(
                                lootMsg
                            );

                            currentState = STATE_DIALOGUE;

                        } else {

                            currentState =
                                STATE_OVERWORLD;
                        }
                    }
                }
            }

            break;

        // ============================================================
        case STATE_VICTORY: {
            // Wait for the player to press a key to exit
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
                currentState = STATE_MAIN_MENU;
            }
            break;
        }
        }
    }
}

// RENDERING
void Game::Draw() {

    BeginDrawing();
    ClearBackground(BgColor);

    if (currentState == STATE_BATTLE) {
        battle.Draw(myPlayer);

    } else if (currentState == STATE_VICTORY) {
        gameRenderer.DrawVictoryScreen(fileScore, playTimer);
        
    } else {

        gameRenderer.DrawFrame(
            currentState,
            myPlayer,
            worldMap,
            dialogueBox,
            fileScore,
            mainMenuSelection,
            playerNameInput,
            leaderboard,
            leaderboardCount);
    }

    EndDrawing();
}

// LOADING LEADERBOARD
void Game::LoadLeaderboard() {
    std::ifstream file("leaderboard.txt");
    leaderboardCount = 0;

    if (file.is_open()) {
        std::string name;
        int score, timeSec;
        
        // Notice we added timeSec here!
        while (file >> name >> score >> timeSec && leaderboardCount < MAX_LEADERBOARD) {
            leaderboard[leaderboardCount].name = name;
            leaderboard[leaderboardCount].score = score;
            leaderboard[leaderboardCount].timeSeconds = timeSec;
            leaderboardCount++;
        }
        file.close();
        SortLeaderboard(); // Automatically sorts whenever loaded
    }
}

// SAVE TO LEADERBOARD
void Game::SaveToLeaderboard() {
    // Load the current top scores
    LoadLeaderboard(); 
    
    // Add our new score
    if (leaderboardCount < MAX_LEADERBOARD) {
        // If the board isn't full yet, just put it at the end
        leaderboard[leaderboardCount].name = myPlayer.GetName();
        leaderboard[leaderboardCount].score = fileScore;
        leaderboard[leaderboardCount].timeSeconds = (int)playTimer;
        leaderboardCount++;
    } else {
        // If the board is full, replace the worst score (at the very bottom) only if theres new best score
        if (fileScore > leaderboard[MAX_LEADERBOARD - 1].score) {
            leaderboard[MAX_LEADERBOARD - 1].name = myPlayer.GetName();
            leaderboard[MAX_LEADERBOARD - 1].score = fileScore;
            leaderboard[MAX_LEADERBOARD - 1].timeSeconds = (int)playTimer;
        }
    }
    
    // Sort the array so the new score bubbles up to its correct rank
    SortLeaderboard();
    
    // Overwrite the file with the newly sorted Top 10 list
    std::ofstream file("leaderboard.txt");
    for (int i = 0; i < leaderboardCount; i++) {
        file << leaderboard[i].name << " " << leaderboard[i].score << " " << leaderboard[i].timeSeconds << "\n";
    }
    file.close();
}

// SORTING LEADERBOARD BY DECENDING ORDER
void Game::SortLeaderboard() {
    // Manual Bubble Sort Implementation (Descending Order)
    for (int i = 0; i < leaderboardCount - 1; i++) {
        for (int j = 0; j < leaderboardCount - i - 1; j++) {
            
            // If the current score is less than the next score, swap them
            if (leaderboard[j].score < leaderboard[j+1].score) {
                ScoreEntry temp = leaderboard[j];
                leaderboard[j] = leaderboard[j+1];
                leaderboard[j+1] = temp;
            }
        }
    }
}
