#include "player.h"

Player::Player(float startX, float startY) {
    position = { startX, startY };
    speed = 200.0f; 
    size = { 20.0f, 12.0f }; // Our tweaked 2.5D collision box
    sprite = LoadTexture("src/sprite/player.png"); 

    // Initialize RPG Stats
    name = "Bober";
    level = 1;
    maxHp = 25;
    hp = 25;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        inventory[i].id = 0;
        inventory[i].name = "Empty";
    }
}

Player::~Player() {
    UnloadTexture(sprite); 
}

void Player::Update(GameMap& map) {
    float deltaTime = GetFrameTime();
    Vector2 moveAmount = { 0.0f, 0.0f };

    // EVENT HANDLER
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) moveAmount.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) moveAmount.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) moveAmount.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveAmount.x += 1.0f;

    // Continuous Diagonal Normalization
    if (moveAmount.x != 0.0f && moveAmount.y != 0.0f) {
        moveAmount.x *= 0.7071f; 
        moveAmount.y *= 0.7071f;
    }

    // Resolve X Axis Movement
    position.x += moveAmount.x * speed * deltaTime;
    if (map.CheckCollision(GetBounds())) {
        position.x -= moveAmount.x * speed * deltaTime; // Backtrack if X hit a wall
    }

    // Resolve Y Axis Movement
    position.y += moveAmount.y * speed * deltaTime;
    if (map.CheckCollision(GetBounds())) {
        position.y -= moveAmount.y * speed * deltaTime; // Backtrack if Y hit a wall
    }
}

// SET HITBOX
Rectangle Player::GetBounds() const {
    // Calculate how far to push the box right so it sits in the middle of the sprite
    float offsetX = (sprite.width - size.x) / 2.0f;
    
    // Calculate how far to push the box down so it sits at the very bottom (the feet)
    float offsetY = sprite.height - size.y; 
    
    return { 
        position.x + offsetX, 
        position.y + offsetY, 
        size.x, 
        size.y 
    };
}

void Player::Draw() {
    // WHITE means "draw the image with its original colors without tinting it"
    DrawTextureV(sprite, position, WHITE);
    
    // DEBUGGING: PLAYER HITBOX
     DrawRectangleLinesEx(GetBounds(), 1, RED);
}

void Player::Teleport(float newX, float newY) {
    position.x = newX;
    position.y = newY;
}

bool Player::AddItem(Item newItem) {
    // Step 1: Loop through the array from slot 0 to 19
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        
        // Step 2: Is this slot completely empty?
        if (inventory[i].id == 0) { 
            
            // Step 3: Put the item here!
            inventory[i] = newItem; 
            return true; // Success! We tell the engine the item was picked up.
        }
    }
    
    // If the loop finishes and we never returned true, the bag is totally full.
    return false; 
}