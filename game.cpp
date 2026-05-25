#include "game.h"
#include <fstream>

// --- INITIALIZATION ---
Game::Game() : myPlayer(388.0f, 256.0f) {
    worldMap.LoadMap("src/levels/level1.txt"); 
    currentState = STATE_OVERWORLD;
    currentEnemy = nullptr;
	std::ifstream inputFile("usersfile.txt");
	inputFile>>fileScore;
	inputFile.close();
	
}

// --- CLEANUP ---
Game::~Game() {
	std::ofstream outputFile("usersfile.txt", std::ios::out);
	outputFile << fileScore ;
	outputFile.close();
    // CloseWindow();
}

// --- MAIN ENGINE LOOP ---
void Game::Run() {
    while (!WindowShouldClose()) {
        ProcessInput();
        Update();
        Draw();
    }
}

// --- INPUT HANDLING ---
void Game::ProcessInput() {
    playerInput = { 0.0f, 0.0f }; // A Vector2 variable stored in your Game class
    
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) playerInput.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) playerInput.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) playerInput.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) playerInput.x += 1.0f;
}

// --- GAME LOGIC ---
void Game::Update() {

    switch (currentState) {
        case STATE_OVERWORLD: {
            myPlayer.Update(worldMap, playerInput);
            worldMap.UpdateEnemies(myPlayer.GetBounds());

            Enemy* touchedEnemy = worldMap.CheckEnemyCollision(myPlayer.GetBounds());
            if (touchedEnemy != nullptr) {
                currentEnemy = touchedEnemy; // Remember who we are fighting
		battle.StartBattle();
                currentState = STATE_BATTLE;
            }

            // CHEST LOGIC
            if (IsKeyPressed(KEY_E)) {
                Chest* nearbyChest = worldMap.CheckChestInteraction(myPlayer.GetBounds());
                if (nearbyChest != nullptr && !nearbyChest->isOpen) {
                    if (myPlayer.AddItem(nearbyChest->content)) {
                        worldMap.MarkChestOpened(nearbyChest); 
                        nearbyChest->isOpen = true; 
			fileScore=fileScore+10;
                    }
                }
            }

            // SIGNPOST LOGIC
            if (IsKeyPressed(KEY_R)) {
                Signpost* nearbySign = worldMap.CheckSignpostInteraction(myPlayer.GetBounds());
                if (nearbySign != nullptr) {
                    dialogueBox.Start(); 
                    for (int i = 0; i < nearbySign->lineCount; i++) {
                        dialogueBox.Enqueue(nearbySign->dialogue[i]);
                    }
                    currentState = STATE_DIALOGUE;
                }
            }

            // PORTAL LOGIC
            Portal hitPortal;
            if (worldMap.CheckPortals(myPlayer.GetBounds(), hitPortal)) {
                worldMap.LoadMap(hitPortal.targetMap);
                myPlayer.Teleport(hitPortal.spawnX, hitPortal.spawnY);
            }
            
		if (IsKeyPressed(KEY_B)) {
			if(currentState==STATE_OVERWORLD){
                    battle.StartBattle();
                    currentState = STATE_BATTLE;
                }
	}
            if (IsKeyPressed(KEY_M)) currentState = STATE_MENU; 
            break;
        }
        case STATE_DIALOGUE: {
            dialogueBox.Update(); 
            if (IsKeyPressed(KEY_SPACE)) {
                if (dialogueBox.IsTextFinished()) {
                    dialogueBox.Dequeue(); 
                    if (!dialogueBox.IsActive()) currentState = STATE_OVERWORLD; 
                } else {
                    dialogueBox.SkipTyping(); 
                }
            }
            break;
        }
        case STATE_MENU:
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) currentState = STATE_OVERWORLD;
            break;
            
case STATE_BATTLE: {
    if (!battle.IsBattleOver()) {
        battle.Update(myPlayer);
    }

    if (battle.IsBattleOver()) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
            if (battle.GetState() == PLAYER_LOSE) {
                worldMap.ResetDefeatedEnemies();
                worldMap.LoadMap("src/levels/level1.txt");
                myPlayer.Teleport(388.0f, 256.0f);
                battle.get_healing() = battle.max_HP();
                currentEnemy = nullptr;
            } else {
                // Player won — mark enemy defeated so it doesn't respawn
                if (currentEnemy != nullptr) {
                    worldMap.MarkEnemyDefeated(currentEnemy);
                    currentEnemy = nullptr;
                }
            }
            currentState = STATE_OVERWORLD;
            battle.StartBattle();
        }
    }
    break;
}
}
}
// --- RENDERING ---
void Game::Draw() {

    BeginDrawing();
    
    if (currentState == STATE_BATTLE) {
            battle.Draw(myPlayer);
     }
    else {
         gameRenderer.DrawFrame(currentState, myPlayer, worldMap);
     }

	if (currentState != STATE_BATTLE){

    DrawRectangle(50, 50, 50, 50, YELLOW);
    DrawRectangle(100, 50, 50, 50, YELLOW);
    DrawRectangle(50, 100, 50, 50, YELLOW);
    DrawRectangle(100, 100, 50, 50, YELLOW);

    DrawRectangleLines(50, 50, 100, 100, BLACK);

    DrawText(
        std::to_string(fileScore).c_str(),
        55,
        60,
        50,
        BLACK
    );
} 

    EndDrawing();
}
