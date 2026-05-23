#include "Battle.h"
#include <cstdlib>
#include <ctime>
#include "Items.h"

BattleSystem::BattleSystem(){
    currentState = PLAYER_TURN;
    playerDefending = false;
    selectedOption = 0;

    enemyMaxHp = 50;
    enemyAttack = 6;

    PlayerMaxHP = 100;
    PlayerHP = PlayerMaxHP;
    PlayerAttack = 10;

    turnTimer = 0.0f;
    turnDelay = 1.0f;

    std::srand(std::time(nullptr));

    StartBattle();
}

void BattleSystem::Player_Damage(Player& player){
    int damage = PlayerAttack + (rand() % 8);
    enemyHp -= damage;

    if (enemyHp <= 0) {
        enemyHp = 0;
        currentState = PLAYER_WIN;
        battleMessage = "";
        return;
    }

    battleMessage = "You attacked!";
    currentState = ENEMY_TURN;
}

void BattleSystem::Enemy_Damage(Player& player){

    int damage = enemyAttack + (rand() % 4);

    if (playerDefending){
        damage /= 2;
        playerDefending = false;
    }

    PlayerHP -= damage;

    if (PlayerHP <= 0){
        PlayerHP = 0;
        currentState = PLAYER_LOSE;
        battleMessage = "You Lose!";
        return;
    }

    battleMessage = "Enemy attacked!";
    currentState = PLAYER_TURN;
}

void BattleItem::Healing(Player& player,BattleSystem& battle){
    int& HP = battle.get_healing();

    if (HPPotion <= 0){
        return;
    }

    HP += HPHeal;

    if (HP > battle.max_HP()){
        HP = battle.max_HP();
    }

    HPPotion--;
}

void BattleSystem::Menu_Option(Player& player){
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)){
        selectedOption--;

        if (selectedOption < 0){selectedOption = MENU_COUNT - 1;}
    }

    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)){
        selectedOption++;

        if (selectedOption >= MENU_COUNT){selectedOption = 0;}
    }

    if (IsKeyPressed(KEY_ENTER)){
        switch (selectedOption){
        case 0:
            Player_Damage(player);
            break;

        case 1:
            playerDefending = true;
            battleMessage = "You defended!";
            currentState = ENEMY_TURN;
            turnTimer = -0.5f;
            break;

        case 2:
            item.Healing(player, *this);
            currentState = ENEMY_TURN;
            break;
        }
    }
}

void BattleSystem::Update(Player& player){

    if (currentState == PLAYER_TURN){
        Menu_Option(player);
        return;
    }

    if (currentState != ENEMY_TURN){
        return;
    }

    turnTimer += GetFrameTime();

    if (turnTimer < turnDelay){
        return;
    }

    Enemy_Damage(player);

    turnTimer = 0.0f;
}

void BattleSystem::StartBattle(){
    currentState = PLAYER_TURN;
    playerDefending = false;
    selectedOption = 0;

    enemyHp = enemyMaxHp;

    battleMessage = "A wild enemy appeared!!";
}

void BattleSystem::Draw(const Player& player){
    ClearBackground(BLACK);

    const int screenWidth = 800;

    DrawText("BATTLE", 320, 30, 40, WHITE);

    DrawRectangleLines(500, 80, 220, 120, WHITE);

    DrawText("ENEMY", 565, 90, 28, WHITE);

    DrawText(
        TextFormat("HP: %d / %d", enemyHp, enemyMaxHp),
        540,
        140,
        24,
        WHITE
    );

    DrawRectangleLines(60, 260, 250, 120, WHITE);

    DrawText("PLAYER", 120, 275, 28, WHITE);

    DrawText(
        TextFormat("HP: %d / %d", PlayerHP, PlayerMaxHP),
        100,
        325,
        24,
        WHITE
    );

    if (currentState == PLAYER_WIN){

        const char* winText = "YOU WON!";
        int textWidth = MeasureText(winText, 40);

        DrawText(
            winText,
            (screenWidth - textWidth) / 2,
            220,
            40,
            WHITE
        );

        int msgWidth = MeasureText(battleMessage.c_str(), 24);

        DrawText(
            battleMessage.c_str(),
            (screenWidth - msgWidth) / 2,
            280,
            24,
            WHITE
        );

        DrawText(
            "Press SPACE to return",
            240,
            340,
            24,
            WHITE
        );

        return;
    }

    if (currentState == PLAYER_LOSE){

        const char* loseText = "YOU LOSE!";
        int textWidth = MeasureText(loseText, 40);

        DrawText(
            loseText,
            (screenWidth - textWidth) / 2,
            220,
            40,
            WHITE
        );

        int msgWidth = MeasureText(battleMessage.c_str(), 24);

        DrawText(
            battleMessage.c_str(),
            (screenWidth - msgWidth) / 2,
            280,
            24,
            WHITE
        );

        DrawText(
            "Press SPACE to return",
            240,
            340,
            24,
            WHITE
        );

        return;
    }

    DrawRectangleLines(100, 430, 600, 70, WHITE);

    int msgWidth = MeasureText(battleMessage.c_str(), 22);

    DrawText(
        battleMessage.c_str(),
        (screenWidth - msgWidth) / 2,
        455,
        22,
        WHITE
    );

    const char* menu[MENU_COUNT] = {
        "ATTACK",
        "DEFEND",
        "POTION"
    };

    for (int i = 0; i < MENU_COUNT; i++){

        Color color = (i == selectedOption)
            ? WHITE
            : GRAY;

        DrawText(
            menu[i],
            140 + (i * 220),
            540,
            28,
            color
        );
    }
}


   bool BattleSystem::IsBattleOver() const {
    return currentState == PLAYER_WIN ||
           currentState == PLAYER_LOSE;
}

    BattleState BattleSystem::GetState() const {
        return currentState;
    }
   
