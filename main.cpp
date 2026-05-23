#include "game.h"
#include "Battle.h"
#include "Items.h"

int main() {
  Game myGame;// Create the game "engine" wrapper
  BattleSystem battle;    
  myGame.Run();    // Start the game loop
  return 0;        // Exit successfully when the loop ends
}
