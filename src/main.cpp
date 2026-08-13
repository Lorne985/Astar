#include <array>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <vector>

#include "Grid.hpp"
#include "raylib.h"

struct OpenNode {
    GridPos pos;
    int fScore;
};

bool operator<(const OpenNode &a, const OpenNode &b) {
    return a.fScore > b.fScore;
}
bool isInsideGrid(const GridPos pos) {
    return 0 <= pos.row && pos.row < ROWS && 0 <= pos.col && pos.col < COLS;
}

std::vector<GridPos> get_neighbors(const Grid<CellState> &grid, GridPos pos) {
    constexpr std::array<GridPos, 4> neighbor = {
        GridPos{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    std::vector<GridPos> res{};
    for (const auto &v : neighbor) {
        GridPos p = pos + v;
        if (isInsideGrid(p) && grid[p.row][p.col] != CellState::Wall)
            res.emplace_back(p);
    }
    return res;
}

void reset_gScore(Grid<int> &score, const std::optional<GridPos> &s) {
    for (auto &scoreRow : score) {
        scoreRow.fill(std::numeric_limits<int>::max());
    }
    if (s != std::nullopt)
        score[s->row][s->col] = 0;
}

void reset_cameFrom(Grid<std::optional<GridPos>> &cameFrom) {
    for (auto &cameFromRow : cameFrom) {
        cameFromRow.fill(std::nullopt);
    }
}

void reset_all_state(Grid<int> &score, Grid<std::optional<GridPos>> &cameFrom,
                     std::priority_queue<OpenNode> &openSet,
                     const std::optional<GridPos> &s,
                     const std::optional<GridPos> &g) {
    reset_gScore(score, s);
    reset_cameFrom(cameFrom);
    openSet = {};
    if (s != std::nullopt && g != std::nullopt) {
        openSet.emplace(s.value(), score[s->row][s->col] +
                                       heuristic(s.value(), g.value()));
    }
}

int main() {
    constexpr int windowWidth = COLS * CELL_SIZE + OFFSET_X;
    constexpr int windowHeight = ROWS * CELL_SIZE + OFFSET_Y;
    InitWindow(windowWidth, windowHeight, "A* Visualizer");

    SetTargetFPS(120);
    //---------------------------------//

    Grid<CellState> grid{};
    Grid<int> gScore{};
    Grid<std::optional<GridPos>> cameFrom;
    reset_gScore(gScore, std::nullopt);
    reset_cameFrom(cameFrom);
    std::optional<GridPos> s = std::nullopt;
    std::optional<GridPos> g = std::nullopt;

    std::priority_queue<OpenNode> openSet{};
    bool blockLeftDrawUntilRelease = false;

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            blockLeftDrawUntilRelease = false;
        }

        bool insideGrid = mouse.x >= OFFSET_X && mouse.y >= OFFSET_Y &&
                          mouse.x < OFFSET_X + COLS * CELL_SIZE &&
                          mouse.y < OFFSET_Y + ROWS * CELL_SIZE;
        if (insideGrid) {
            int col = (mouse.x - OFFSET_X) / CELL_SIZE;
            int row = (mouse.y - OFFSET_Y) / CELL_SIZE;

            const bool leftPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
            const bool leftDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
            const bool rightDown = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);

            const bool placeStart = leftPressed && IsKeyDown(KEY_S);
            const bool placeGoal = leftPressed && IsKeyDown(KEY_G);

            if (placeStart) {
                blockLeftDrawUntilRelease = true;
                if (!(g != std::nullopt && g == GridPos{row, col})) {
                    s = {row, col};
                    reset_all_state(gScore, cameFrom, openSet, s, g);
                    grid[row][col] = CellState::Empty;
                }
            } else if (placeGoal) {
                blockLeftDrawUntilRelease = true;
                if (!(s != std::nullopt && s == GridPos{row, col})) {
                    g = {row, col};
                    reset_all_state(gScore, cameFrom, openSet, s, g);
                    grid[row][col] = CellState::Empty;
                }
            } else if (leftDown && !blockLeftDrawUntilRelease &&
                       s != GridPos{row, col} && g != GridPos{row, col}) {
                reset_all_state(gScore, cameFrom, openSet, s, g);
                grid[row][col] = CellState::Wall;
            } else if (rightDown) {
                if (s != std::nullopt && s == GridPos{row, col}) {
                    s = std::nullopt;
                }
                if (g != std::nullopt && g == GridPos{row, col}) {
                    g = std::nullopt;
                }
                grid[row][col] = CellState::Empty;
                reset_all_state(gScore, cameFrom, openSet, s, g);
            }
        }

        BeginDrawing();

        ClearBackground(BLACK);
        DrawText("Left: draw wall     Right: erase", OFFSET_X, 20, 20,
                 RAYWHITE);

        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                Rectangle rect{static_cast<float>(OFFSET_X + col * CELL_SIZE),
                               static_cast<float>(OFFSET_Y + row * CELL_SIZE),
                               static_cast<float>(CELL_SIZE),
                               static_cast<float>(CELL_SIZE)};

                Color c = GetCellColor(grid[row][col]);
                if (s != std::nullopt && row == s->row && col == s->col) {
                    c = GREEN;
                } else if (g != std::nullopt && row == g->row &&
                           col == g->col) {
                    c = RED;
                }
                DrawRectangleRec(rect, c);
                DrawRectangleLinesEx(rect, 1.0F, GRAY);
            }
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
