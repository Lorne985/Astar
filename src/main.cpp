#include <array>

#include "Grid.hpp"
#include "raylib.h"



using Grid = std::array<std::array<CellState, COLS>, ROWS>;
int main() {
  constexpr int windowWidth = COLS * CELL_SIZE + OFFSET_X;
  constexpr int windowHeight = ROWS * CELL_SIZE + OFFSET_Y;
  InitWindow(windowWidth, windowHeight, "A* Visualizer");

  SetTargetFPS(120);
  Grid grid{};
  GridPosition s{0, 0};
  GridPosition g{0, 0};
  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();

    bool insideGrid = mouse.x >= OFFSET_X && mouse.y >= OFFSET_Y &&
                      mouse.x < OFFSET_X + COLS * CELL_SIZE &&
                      mouse.y < OFFSET_Y + ROWS * CELL_SIZE;
    if (insideGrid) {
        int col = (mouse.x - OFFSET_X) / CELL_SIZE;
        int row = (mouse.y - OFFSET_Y) / CELL_SIZE;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (IsKeyDown(KEY_S)) {
                grid[s.row][s.col] = CellState::Empty;
                grid[row][col] = CellState::Start;
                s = {row, col};
            }else if (IsKeyDown(KEY_G)){
                grid[g.row][g.col] = CellState::Empty;
                grid[row][col] = CellState::Goal;
                g = {row, col};
            }else {
                grid[row][col] = CellState::Wall;
            }
        }
        // if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        //     grid[row][col] = CellState::Wall;
        // }
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            grid[row][col] = CellState::Empty;
        }

    }
    BeginDrawing();

    
    ClearBackground(BLACK);
    DrawText("Left: draw wall     Right: erase", OFFSET_X, 20, 20, RAYWHITE);

    for(int row = 0; row < ROWS; row++) {
        for(int col = 0; col < COLS; col++) {
            Rectangle rect{static_cast<float>(OFFSET_X + col * CELL_SIZE),
                static_cast<float>(OFFSET_Y + row * CELL_SIZE),
                static_cast<float>(CELL_SIZE),
                static_cast<float>(CELL_SIZE)
            };

            DrawRectangleRec(rect, GetCellColor(grid[row][col]));
            DrawRectangleLinesEx(rect, 1.0F, GRAY);
        }
    }

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
