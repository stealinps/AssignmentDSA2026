#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
#include "map.h"
#include <string>

class Player {
private:
    Vector2 position;
    float speed;
    Vector2 size;     
    Texture2D sprite; 

    // PLAYERS DATA
    std::string name;
    int level;
    int hp;
    int maxHp;

public:
    Player(float startX, float startY);
    ~Player();        
    
    void Update(GameMap& map);
    void Draw();
    Rectangle GetBounds() const;
    // Add this inside the public section of your Player class
    void Teleport(float newX, float newY);

    // Getters - Read-only access to private variables
    std::string GetName() const { return name; }
    int GetLevel() const { return level; }
    int GetHP() const { return hp; }
    int GetMaxHP() const { return maxHp; }
};

#endif