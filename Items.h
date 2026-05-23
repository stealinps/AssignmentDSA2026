
#ifndef ITEMS_H
#define ITEMS_H

#include "Battle.h"
#include "raylib.h"
#include "player.h"
#include <string>

class BattleSystem;
class BattleItem{
    private:
        int HPHeal = 20;
        int HPPotion = 3;

    public:
        void Healing(Player& player, BattleSystem& battle);
};

#endif