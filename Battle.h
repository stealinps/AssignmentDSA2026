#ifndef BATTLE_H
#define BATTLE_H

#include "raylib.h"
#include "player.h"
#include "Items.h"
#include <string>

enum BattleState {
    PLAYER_TURN,
    ENEMY_TURN,
    PLAYER_WIN,
    PLAYER_LOSE
};

enum MenuOption {ATTACK, ITEMS, DEFEND,MENU_COUNT};



class BattleSystem{
    private:
    std::string enemyname;
    int enemyHp;
    int enemyMaxHp;
    int enemyAttack;
    int PlayerHP;
    int PlayerMaxHP;
    int PlayerAttack;
    BattleItem item;

    BattleState currentState;
    std::string battleMessage;

    bool playerDefending;
    int selectedOption = ATTACK;

    float turnTimer;
    float turnDelay;

    public:
        BattleSystem();
        int& get_healing() {return PlayerHP;};
        int max_HP() {return PlayerMaxHP;};
        void StartBattle();
        void Update(Player& player);
        void Enemy_Damage(Player& player);
        void Player_Damage(Player& player);
        void Draw(const Player& player);
        void Menu_Option(Player& player);

        bool IsBattleOver() const;

        BattleState GetState() const;
};

#endif
