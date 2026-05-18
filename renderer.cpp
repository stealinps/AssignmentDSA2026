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
    BeginDrawing();
    
    ClearBackground(BgColor);

    switch (state) {
        case STATE_OVERWORLD:
            DrawOverworld(player, map);
            break;
        case STATE_MENU:
            DrawOverworld(player, map);
            DrawMenu(player); 
            break;
        case STATE_BATTLE:
            DrawBattle();
            break;
    }

    EndDrawing();
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
    
    DrawText("Press 'M' to close", panelX + 20, panelY + 250, 15, LIGHTGRAY);
}