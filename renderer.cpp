#include "renderer.h"

Renderer::Renderer() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Adding a Background");
    SetTargetFPS(60);

    SetExitKey(0); // ESC key wont close the game

    bgTexture = LoadTexture("src/sprite/background.png"); 
}

Renderer::~Renderer() {
    // UNLOAD THE BACKGROUND
    UnloadTexture(bgTexture);
    CloseWindow();
}

void Renderer::DrawFrame(GameState state, Player& player, GameMap& map) {

    switch (state) {
        case STATE_OVERWORLD:
            DrawOverworld(player, map);
            break;
        case STATE_DIALOGUE: // <--- ADD THIS RIGHT BELOW OVERWORLD!
            map.Draw();
            player.Draw();
            break;
        case STATE_MENU:
            DrawOverworld(player, map);
            DrawMenu(player); 
            break;
        case STATE_BATTLE:
            DrawBattle();
            break;
    }
}

void Renderer::DrawOverworld(Player& player, GameMap& map) {

    for (int y = 0; y < SCREEN_HEIGHT; y += bgTexture.height) {
        for (int x = 0; x < SCREEN_WIDTH; x += bgTexture.width) {
            DrawTexture(bgTexture, x, y, WHITE);
        }
    }

    // Draw the floor grid and the wall tiles
    // (Note: If your background covers the whole screen, remove DrawGrid() so it doesn't draw lines over the bg)
    //DrawGrid(); 
    map.Draw();    

    // Draw the player on top of the world
    player.Draw(); 
    
    DrawText("OVERWORLD: WASD to move. M for menu, B for battle(unfinished)", 10, 10, 15, RAYWHITE);
}

// UNFINISHED
void Renderer::DrawBattle() {
    DrawRectangle(600, 100, 64, 64, EnemyColor); 
    DrawText("BATTLE ENGAGED! Press 'ESC' to flee.", 200, 250, 20, RAYWHITE);
}

void Renderer::DrawGrid() {
    for (int i = 0; i < SCREEN_WIDTH / TILE_SIZE; i++) {
        DrawLine(i * TILE_SIZE, 0, i * TILE_SIZE, SCREEN_HEIGHT, LIGHTGRAY);
    }
    for (int i = 0; i < SCREEN_HEIGHT / TILE_SIZE; i++) {
        DrawLine(0, i * TILE_SIZE, SCREEN_WIDTH, i * TILE_SIZE, LIGHTGRAY);
    }
}

void Renderer::DrawMenu(const Player& player) {
    // Draw a translucent background panel on the right side of the screen
    int panelX = SCREEN_WIDTH - 250;
    int panelY = 50;
    int panelWidth = 200;
    int panelHeight = 300;
    
    DrawRectangle(panelX, panelY, panelWidth, panelHeight, MenuPanelColor);
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, RAYWHITE); // A nice white border

    // Format the text using C++ strings
    std::string nameText = "NAME: " + player.GetName();
    std::string levelText = "LVL: " + std::to_string(player.GetLevel());
    std::string hpText = "HP: " + std::to_string(player.GetHP()) + " / " + std::to_string(player.GetMaxHP());

    // Draw the text to the screen using Raylib
    // .c_str() converts a modern C++ string back into older C-style text for Raylib
    DrawText(nameText.c_str(), panelX + 20, panelY + 30, 20, RAYWHITE);
    DrawText(levelText.c_str(), panelX + 20, panelY + 70, 20, RAYWHITE);
    DrawText(hpText.c_str(), panelX + 20, panelY + 110, 20, GREEN);

    DrawText("INVENTORY:", panelX + 20, panelY + 160, 20, GOLD);

    // Loop through the fixed array to display items
    int drawY = panelY + 190;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        Item currentItem = player.GetInventoryItem(i);
        
        if (currentItem.id != 0) { // Only draw slots that are NOT empty
            // .c_str() converts the string for Raylib
            DrawText(currentItem.name.c_str(), panelX + 20, drawY, 15, RAYWHITE);
            drawY += 25; // Move the text cursor down for the next item
        }
    }
    
    DrawText("Press 'M' to close", panelX + 20, panelY + 250, 15, LIGHTGRAY);
}