#include <iostream>
#include <raylib.h>

#define COLS 25
#define ROWS 25

enum Tile { FLOOR, WALL };

Tile grid[ROWS][COLS];

void InitGrid() {
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            grid[y][x] = FLOOR;
}
void DrawGrid(int cellSize) {
	for (int y = 0; y < ROWS; y++) {
		for (int x = 0; x < COLS; x++) {
			Color c;
			if(grid[y][x]==WALL) c=DARKGRAY;
			else c=LIGHTGRAY ;
			DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, c);
			DrawRectangleLines(x * cellSize, y * cellSize, cellSize, cellSize, GRAY);
        }
    }
}

struct Player {
	int x = 1, y = 1;  // grid coordinates, not pixels
	void Draw(int cellSize) {
	DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, BLUE);
}
	void HandleInput() {
		if (IsKeyPressed(KEY_W) && y > 0)          y--;
		if (IsKeyPressed(KEY_S) && y < ROWS - 1)   y++;
		if (IsKeyPressed(KEY_A) && x > 0)           x--;
		if (IsKeyPressed(KEY_D) && x < COLS - 1)    x++;
	}
};



int main(){
	const int cellSize = 30; 
	Color darkBlue={44,44,127,255};
	int winwidth=720;
	int winheight=480;
	InitWindow(winwidth,winheight,"Project DSA");
	SetTargetFPS(60);
	Player player;

	InitGrid();
	grid[5][5] = WALL;  // test wall

	while(WindowShouldClose()==false){
		player.HandleInput(); 
		BeginDrawing();
		ClearBackground(BLACK);
		DrawGrid(cellSize);
		player.Draw(cellSize);
		EndDrawing();
	}
	CloseWindow();

}
