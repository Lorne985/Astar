#pragma once

#include "raylib.h"
#include <cstdlib>
#include <array>

constexpr int ROWS = 20;
constexpr int COLS = 30;
constexpr int CELL_SIZE = 40;
constexpr int OFFSET_X = 0;
constexpr int OFFSET_Y = 60;

enum class CellState { Empty, Wall};

template<typename T>
using Grid = std::array<std::array<T, COLS>, ROWS>;


inline Color GetCellColor(CellState state) {
    switch (state) {
    case CellState::Wall:
        return DARKGRAY;
    default:
        return RAYWHITE;
    }
}

struct GridPos {
    int row, col;
};

inline bool operator==(const GridPos &g1, const GridPos &g2) {
    return g1.row == g2.row && g1.col == g2.col;
}
inline bool operator!=(const GridPos &g1, const GridPos &g2) {
    return g1.row != g2.row || g1.col != g2.col;
}
inline GridPos operator+(const GridPos &g1, const GridPos &g2) {
    return GridPos{g1.row + g2.row ,g1.col + g2.col};
}

inline int heuristic(const GridPos& from, const GridPos& to) {
    return std::abs(from.row - to.row) + std::abs(from.col - to.col);
}