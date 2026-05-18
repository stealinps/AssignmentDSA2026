#ifndef CONFIG_H
#define CONFIG_H
#include "raylib.h"
#include <iostream>

#define TILE_SIZE 32
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 576

struct Item {
    int id;           // 0 = Empty Slot. Anything else is a real item.
    std::string name; // e.g., "Health Potion", "Iron Key"
};

enum GameState {
    STATE_OVERWORLD,
    STATE_BATTLE,
    STATE_MENU,
    STATE_DIALOGUE
};

static const Color BgColor = {30, 30, 46, 255};     
static const Color PlayerColor = {203, 166, 247, 255}; 
static const Color EnemyColor = {243, 139, 168, 255};  
static const Color MenuPanelColor = {24, 24, 37, 220}; // NEW: 220 makes it slightly see-through!

#endif