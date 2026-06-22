#include "Battle.h"
#include "config.h"
#include "audio.h"
#include <cstdlib>
#include <ctime>
 
// ------------------------------------------------------------
// CONSTRUCTOR
// ------------------------------------------------------------
BattleSystem::BattleSystem() {
    currentState  = PLAYER_TURN;
    playerDefending = false;
    selectedOption  = 0;
    itemMenuOpen    = false;
    itemSubMenuOption = 0;
    isBossBattle    = false;
 
    enemyMaxHp  = 50;
    enemyAttack = 6;
 
    PlayerMaxHP  = 100;
    PlayerHP     = PlayerMaxHP;
    PlayerAttack = 10;
 
    turnTimer = 0.0f;
    turnDelay = 1.0f;
 
    playerStrengthEffect = PotionEffect(EFFECT_NONE, 0, 0);
    playerDefenseEffect  = PotionEffect(EFFECT_NONE, 0, 0);
 
    std::srand(std::time(nullptr));
 
    currentExpReward = 0;
    currentScoreReward = 0;
    StartBattle("Unknown Glitch", 50, 6, 0, 0);
}
 
// ------------------------------------------------------------
// COMBAT ACTIONS
// ------------------------------------------------------------
void BattleSystem::Player_Damage(Player& player) {
    AudioManager::PlayHitSound();
    int damage = GetActualPlayerDamage() + (rand() % 8);
    enemyHp -= damage;
 
    if (enemyHp <= 0) {
        enemyHp = 0;
        currentState  = PLAYER_WIN;
        player.GainExperience(currentExpReward);
        player.AddScore(currentScoreReward);
        battleMessage = "You defeated the enemy! Gained " + std::to_string(currentExpReward) + " EXP!";
        return;
    }
 
    battleMessage = "You attacked for " + std::to_string(damage) + " damage!";
    currentState  = ENEMY_TURN;
}
 
void BattleSystem::Enemy_Damage(Player& player) {
    AudioManager::PlayHitSound();
    int damage = enemyAttack + (rand() % 4);
 
    if (playerDefending) {
        damage /= 2;
        playerDefending = false;
    }
 
    // Defense potion: 15% damage reduction while active
    if (playerDefenseEffect.IsActive()) {
        damage -= (damage * 15) / 100;
    }
 
    PlayerHP -= damage;
 
    if (PlayerHP <= 0) {
        PlayerHP     = 0;
        currentState  = PLAYER_LOSE;
        battleMessage = "You Lose!";
        return;
    }
 
    battleMessage = "Enemy attacked for " + std::to_string(damage) + "!";
    currentState  = PLAYER_TURN;
}
 
// ------------------------------------------------------------
// POTION EFFECTS  (each checks inventory, uses 1 charge, applies effect)
// BUG FIX: Previously used a hardcoded BattleItem counter that was
// completely disconnected from the actual Inventory linked list.
// ------------------------------------------------------------
void BattleSystem::ApplyHealthPotion(Player& player) {
    if (player.GetItemQuantity(ITEM_HEALTH_POTION) <= 0) {
        battleMessage = "No HP Potions left!";
        return;
    }
    player.UseItem(ITEM_HEALTH_POTION);
    PlayerHP += 20;
    if (PlayerHP > PlayerMaxHP) PlayerHP = PlayerMaxHP;
    battleMessage = "Used HP Potion! Restored 20 HP!";
    itemMenuOpen  = false;
    currentState  = ENEMY_TURN;
    UpdatePotionEffects();
}
 
void BattleSystem::ApplyStrengthPotion(Player& player) {
    if (player.GetItemQuantity(ITEM_STRENGTH_POTION) <= 0) {
        battleMessage = "No Strength Potions left!";
        return;
    }
    player.UseItem(ITEM_STRENGTH_POTION);
    playerStrengthEffect = PotionEffect(EFFECT_STRENGTH,
                                        STRENGTH_POTION_DURATION,
                                        STRENGTH_POTION_DAMAGE_BONUS);
    battleMessage = "Strength Potion! +" +
                    std::to_string(STRENGTH_POTION_DAMAGE_BONUS) +
                    " damage for " +
                    std::to_string(STRENGTH_POTION_DURATION) + " turns!";
    itemMenuOpen  = false;
    currentState  = ENEMY_TURN;
    UpdatePotionEffects();
}
 
void BattleSystem::ApplyDefensePotion(Player& player) {
    if (player.GetItemQuantity(ITEM_DEFENSE_POTION) <= 0) {
        battleMessage = "No Defense Potions left!";
        return;
    }
    player.UseItem(ITEM_DEFENSE_POTION);
    PlayerMaxHP += DEFENSE_POTION_HP_BONUS;
    PlayerHP    += DEFENSE_POTION_HP_BONUS;
    if (PlayerHP > PlayerMaxHP) PlayerHP = PlayerMaxHP;
    playerDefenseEffect = PotionEffect(EFFECT_DEFENSE, 5, DEFENSE_POTION_HP_BONUS);
    battleMessage = "Defense Potion! +" +
                    std::to_string(DEFENSE_POTION_HP_BONUS) + " Max HP!";
    itemMenuOpen  = false;
    currentState  = ENEMY_TURN;
    UpdatePotionEffects();
}
 
void BattleSystem::UpdatePotionEffects() {
    if (playerStrengthEffect.IsActive()) {
        playerStrengthEffect.DecreaseDuration();
        if (!playerStrengthEffect.IsActive()) {
            battleMessage = "Strength effect wore off!";
        }
    }
    if (playerDefenseEffect.IsActive()) {
        playerDefenseEffect.DecreaseDuration();
    }
}
 
int BattleSystem::GetActualPlayerDamage() const {
    int base = PlayerAttack;
    if (playerStrengthEffect.IsActive()) base += playerStrengthEffect.magnitude;
    return base;
}
 
int BattleSystem::GetActualPlayerMaxHP() const {
    return PlayerMaxHP;
}
 
// ------------------------------------------------------------
// MENU INPUT
// ------------------------------------------------------------
 
// Handles A/D/ENTER/ESC inside the item sub-menu
void BattleSystem::HandleItemSubMenu(Player& player) {
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        itemSubMenuOption--;
        if (itemSubMenuOption < 0) itemSubMenuOption = ITEM_SUBMENU_COUNT - 1;
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        itemSubMenuOption++;
        if (itemSubMenuOption >= ITEM_SUBMENU_COUNT) itemSubMenuOption = 0;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        itemMenuOpen = false;
        return;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        switch (itemSubMenuOption) {
            case 0: ApplyHealthPotion(player);   break;
            case 1: ApplyStrengthPotion(player); break;
            case 2: ApplyDefensePotion(player);  break;
        }
    }
}
 
// Main battle menu (ATTACK / DEFEND / ITEMS)
// BUG FIX: Switch cases now match corrected enum (ATTACK=0, DEFEND=1, ITEMS=2).
// Old code had ATTACK=0 correct, but case 1 ran defend logic while enum said
// ITEMS=1, and case 2 ran heal logic while enum said DEFEND=2.
void BattleSystem::Menu_Option(Player& player) {
    // Route all input to the item sub-menu when it is open
    if (itemMenuOpen) {
        HandleItemSubMenu(player);
        return;
    }
 
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = MENU_COUNT - 1;
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        selectedOption++;
        if (selectedOption >= MENU_COUNT) selectedOption = 0;
    }
 
    if (IsKeyPressed(KEY_ENTER)) {
        switch (selectedOption) {
            case ATTACK:
                Player_Damage(player);
                UpdatePotionEffects();
                break;
 
            case DEFEND:
                playerDefending = true;
                battleMessage   = "You braced for impact!";
                currentState    = ENEMY_TURN;
                turnTimer       = -0.5f;  // Small extra delay so it feels deliberate
                UpdatePotionEffects();
                break;
 
            case ITEMS:
                itemMenuOpen      = true;
                itemSubMenuOption = 0;
                battleMessage     = "Choose an item. (ESC to cancel)";
                break;
        }
    }
}
 
// ------------------------------------------------------------
// UPDATE
// ------------------------------------------------------------
void BattleSystem::Update(Player& player) {
    if (currentState == PLAYER_TURN) {
        Menu_Option(player);
        return;
    }
    if (currentState != ENEMY_TURN) return;
 
    turnTimer += GetFrameTime();
    if (turnTimer < turnDelay) return;
 
    if (isBossBattle) BossTurn(player);
    else              Enemy_Damage(player);
    turnTimer = 0.0f;
}
 
// ------------------------------------------------------------
// START / RESET
// ------------------------------------------------------------
void BattleSystem::StartBattle(std::string name, int maxHp, int attack, int expReward, int scoreReward) {
    currentState      = PLAYER_TURN;
    playerDefending   = false;
    selectedOption    = 0;
    itemMenuOpen      = false;   // Always close sub-menu on battle start
    itemSubMenuOption = 0;
    isBossBattle      = false;   // ordinary enemy

    enemyname     = name;
    enemyMaxHp    = maxHp;
    enemyHp       = maxHp;
    enemyAttack   = attack;
    
    currentExpReward = expReward;
    currentScoreReward = scoreReward;
    
    
    // Personalize the entry message
    battleMessage = "A wild " + enemyname + " appeared!";
 
    playerStrengthEffect = PotionEffect(EFFECT_NONE, 0, 0);
    playerDefenseEffect  = PotionEffect(EFFECT_NONE, 0, 0);
}

// ------------------------------------------------------------
// FINAL BOSS — the enemy turn is driven by the Boss AI, which
// uses a hand-built Queue (attack plan), Stack (rage charges) and
// binary decision Tree (move selection) to fight back.
// ------------------------------------------------------------
void BattleSystem::StartBossBattle(std::string name, int maxHp, int attack, int expReward, int scoreReward) {
    currentState      = PLAYER_TURN;
    playerDefending   = false;
    selectedOption    = 0;
    itemMenuOpen      = false;
    itemSubMenuOption = 0;
    isBossBattle      = true;
    enemyname     = name;
    enemyMaxHp    = maxHp;
    enemyHp       = maxHp;
    enemyAttack   = attack; // Boss damage mostly comes from its AI tree, but good to store
    
    currentExpReward = expReward;
    currentScoreReward = scoreReward;
    battleMessage = enemyname + " awakens. It will attack with Stacks, Trees and Queues!";

    boss.Reset();                 // empty the attack queue + rage stack

    playerStrengthEffect = PotionEffect(EFFECT_NONE, 0, 0);
    playerDefenseEffect  = PotionEffect(EFFECT_NONE, 0, 0);
}

// One boss turn: ask the Boss AI what it does, then apply the damage here
// (so DEFEND and the defense potion are handled in one place).
void BattleSystem::BossTurn(Player& player) {
    (void)player;  // player stats are read via the members below

    int bossPct   = (enemyMaxHp > 0) ? (enemyHp * 100) / enemyMaxHp : 0;
    int maxHp     = GetActualPlayerMaxHP();
    int playerPct = (maxHp > 0) ? (PlayerHP * 100) / maxHp : 0;

    BossDecision dec = boss.TakeTurn(bossPct, playerPct, playerDefending);

    int damage = dec.damage;

    // DEFEND halves damage — unless the move explicitly pierces guard.
    if (playerDefending) {
        if (!dec.ignoresDefend) damage /= 2;
        playerDefending = false;
    }

    // Defense potion: 15% damage reduction while active.
    if (playerDefenseEffect.IsActive()) {
        damage -= (damage * 15) / 100;
    }

    battleMessage = dec.text;

    if (damage > 0) {
        PlayerHP -= damage;
        if (PlayerHP <= 0) {
            PlayerHP      = 0;
            currentState  = PLAYER_LOSE;
            battleMessage = "The Compiler crashed you. You Lose!";
            return;
        }
    }

    currentState = PLAYER_TURN;
}
 
// ------------------------------------------------------------
// DRAW
// ------------------------------------------------------------
void BattleSystem::Draw(const Player& player) {
    ClearBackground(BLACK);
 
    const int screenW = 800;
 
    // --- Title ---
    DrawText(isBossBattle ? "BOSS BATTLE" : "BATTLE", isBossBattle ? 250 : 320, 30, 40, WHITE);
 
    // --- Enemy panel ---
    DrawRectangleLines(500, 80, 220, 120, WHITE);
    if (isBossBattle) {
        DrawText(enemyname.c_str(), 512, 90, 24, EnemyColor); // Or whatever custom color you had
    } else {
        // We now use enemyname instead of the hardcoded "ENEMY" text
        DrawText(enemyname.c_str(), 512, 90, 24, WHITE);
    }
    DrawText(TextFormat("HP: %d / %d", enemyHp, enemyMaxHp), 540, 140, 24, WHITE);

    // --- Boss telegraph: upcoming QUEUE + rage STACK ---
    // Lets the player read what the data structures are doing.
    if (isBossBattle &&
        currentState != PLAYER_WIN && currentState != PLAYER_LOSE) {

        DrawText(TextFormat("RAGE STACK: %d charge(s)", boss.ChargeCount()),
                 500, 205, 16, ORANGE);

        DrawText("INCOMING (queue):", 500, 228, 16, SKYBLUE);
        int qn = boss.QueuedCount();
        if (qn == 0) {
            DrawText("(planning...)", 510, 248, 16, GRAY);
        } else {
            std::string line = "";
            for (int i = 0; i < qn; i++) {
                if (i > 0) line += " > ";
                line += boss.PeekLabel(i);
            }
            DrawText(line.c_str(), 510, 248, 16, WHITE);
        }
    }
 
    // --- Player panel ---
    DrawRectangleLines(60, 260, 250, 120, WHITE);
    DrawText("PLAYER", 120, 275, 28, WHITE);
    DrawText(TextFormat("HP: %d / %d", PlayerHP, GetActualPlayerMaxHP()), 100, 325, 24, WHITE);
 
    // --- Active potion effects (shown under player panel) ---
    int effectY = 400;
    if (playerStrengthEffect.IsActive()) {
        DrawText(TextFormat("STR: +%d dmg (%d turns)",
                            playerStrengthEffect.magnitude,
                            playerStrengthEffect.duration),
                 70, effectY, 18, YELLOW);
        effectY += 22;
    }
    if (playerDefenseEffect.IsActive()) {
        DrawText(TextFormat("DEF: +%d max HP active", playerDefenseEffect.magnitude),
                 70, effectY, 18, SKYBLUE);
    }
 
    // --- Win / Lose screens ---
    if (currentState == PLAYER_WIN) {
        const char* winText = "YOU WON!";
        DrawText(winText, (screenW - MeasureText(winText, 40)) / 2, 200, 40, WHITE);
        DrawText(battleMessage.c_str(),
                 (screenW - MeasureText(battleMessage.c_str(), 22)) / 2,
                 260, 22, GOLD);
        DrawText("Press SPACE to continue", 240, 310, 22, GRAY);
        return;
    }
    if (currentState == PLAYER_LOSE) {
        const char* loseText = "YOU LOSE!";
        DrawText(loseText, (screenW - MeasureText(loseText, 40)) / 2, 200, 40, RED);
        DrawText(battleMessage.c_str(),
                 (screenW - MeasureText(battleMessage.c_str(), 22)) / 2,
                 260, 22, WHITE);
        DrawText("Press SPACE to restart", 240, 310, 22, GRAY);
        return;
    }
 
    // --- Message box ---
    DrawRectangleLines(100, 440, 600, 60, WHITE);
    DrawText(battleMessage.c_str(),
             (screenW - MeasureText(battleMessage.c_str(), 20)) / 2,
             462, 20, WHITE);
 
    // --- Item sub-menu (drawn above message box when open) ---
    if (itemMenuOpen) {
        DrawRectangle(100, 390, 600, 46, {20, 20, 50, 230});
        DrawRectangleLines(100, 390, 600, 46, YELLOW);
        DrawText("ITEMS:", 115, 395, 16, YELLOW);
 
        const char* itemLabels[]  = { "HP Potion", "Str Potion", "Def Potion" };
        const int   itemIDs[]     = { ITEM_HEALTH_POTION, ITEM_STRENGTH_POTION, ITEM_DEFENSE_POTION };
 
        for (int i = 0; i < ITEM_SUBMENU_COUNT; i++) {
            int qty = player.GetItemQuantity(itemIDs[i]);
            Color col;
            if (i == itemSubMenuOption) {
                col = (qty > 0) ? YELLOW : DARKGRAY;
            } else {
                col = (qty > 0) ? WHITE : DARKGRAY;
            }
            DrawText(TextFormat("%s x%d", itemLabels[i], qty),
                     180 + (i * 175), 400, 16, col);
        }
        DrawText("ESC cancel", 595, 420, 13, GRAY);
    }
 
    // --- Main menu (ATTACK / DEFEND / ITEMS) ---
    const char* menuLabels[MENU_COUNT] = { "ATTACK", "DEFEND", "ITEMS" };
    for (int i = 0; i < MENU_COUNT; i++) {
        Color col = (i == selectedOption && !itemMenuOpen) ? WHITE : GRAY;
        DrawText(menuLabels[i], 140 + (i * 220), 520, 28, col);
    }
}
 
// ------------------------------------------------------------
// STATE QUERIES
// ------------------------------------------------------------
bool BattleSystem::IsBattleOver() const {
    return currentState == PLAYER_WIN || currentState == PLAYER_LOSE;
}
 
BattleState BattleSystem::GetState() const {
    return currentState;
}

void BattleSystem::ResetPlayerStats() {
    PlayerMaxHP = 100;
    PlayerHP = 100;
    PlayerAttack = 10;
}