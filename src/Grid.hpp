#pragma once

#include "raylib.h"
constexpr int ROWS = 20;
constexpr int COLS = 30;
constexpr int CELL_SIZE = 40;
constexpr int OFFSET_X = 0;
constexpr int OFFSET_Y = 60;

enum class CellState {
    Empty,
    Wall,
    Start,
    Goal,
    Open,
    Closed,
    Path
};

struct GridPosition {
    int row, col;
};


Color GetCellColor(CellState state) {
  switch (state) {
  case CellState::Wall:
    return DARKGRAY;
  case CellState::Start:
    return GREEN;
  case CellState::Goal:
    return RED;
  case CellState::Open:
    return YELLOW;
  case CellState::Closed:
    return SKYBLUE;
  case CellState::Path:
    return PURPLE;
  default:
    return RAYWHITE;
  }
}

