#include <array>
#include <limits>
#include <optional>
#include <queue>
#include <vector>

#include "Grid.hpp"
#include "raylib.h"

struct OpenNode {
    GridPos pos;
    int fScore;
    int hScore;
};

enum class SearchPeriod {
    Unready,
    Searching,
    Found,
    NoPath,
};

bool operator<(const OpenNode &a, const OpenNode &b) {
    if (a.fScore == b.fScore) {
        return a.hScore > b.hScore;
    }
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

void reset_grid_bool(Grid<bool> &grid_bool) {
    for (auto &grid_row : grid_bool) {
        grid_row.fill(false);
    }
}
void reset_all_state(Grid<int> &score, Grid<std::optional<GridPos>> &cameFrom,
                     std::priority_queue<OpenNode> &openSet, Grid<bool> &closed,Grid<bool>& path,
                     const std::optional<GridPos> &s,
                     const std::optional<GridPos> &g) {
    reset_gScore(score, s);
    reset_cameFrom(cameFrom);
    reset_grid_bool(closed);
    reset_grid_bool(path);
    openSet = {};
    if (s != std::nullopt && g != std::nullopt) {
        int hs = heuristic(s.value(), g.value());
        openSet.emplace(s.value(), score[s->row][s->col] + hs, hs);
    }
}

int main() {
    constexpr int windowWidth = COLS * CELL_SIZE + OFFSET_X;
    constexpr int windowHeight = ROWS * CELL_SIZE + OFFSET_Y;
    InitWindow(windowWidth, windowHeight, "A* Visualizer");

    SetTargetFPS(120);
    //---------------------------------//

    SearchPeriod sp = SearchPeriod::Unready;
    Grid<CellState> grid{};
    Grid<int> gScore{};
    Grid<std::optional<GridPos>> cameFrom;
    Grid<bool> closed{};
    Grid<bool> path{};
    reset_gScore(gScore, std::nullopt);
    reset_cameFrom(cameFrom);
    reset_grid_bool(closed);
    reset_grid_bool(path);
    std::optional<GridPos> s = std::nullopt;
    std::optional<GridPos> g = std::nullopt;

    std::priority_queue<OpenNode> openSet{};

    bool blockLeftDrawUntilRelease = false;
    bool showScores = false;

    // Timer
    float searchTimer = 0.f;
    constexpr float searchInterval = 0.1f;

    while (!WindowShouldClose()) {
        searchTimer += GetFrameTime();
        if (IsKeyPressed(KEY_D)) {
            showScores = !showScores;
        }
        if (IsKeyPressed(KEY_C)) {
            s = std::nullopt;
            g = std::nullopt;
            for (auto& grid_row: grid) {
                grid_row.fill(CellState::Empty);
            }
            reset_all_state(gScore, cameFrom, openSet, closed, path, s, g);
        }
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

            bool state_change = false;
            if (placeStart) {
                blockLeftDrawUntilRelease = true;
                if (!(g != std::nullopt && g == GridPos{row, col})) {
                    s = {row, col};
                    grid[row][col] = CellState::Empty;
                    state_change = true;
                }
            } else if (placeGoal) {
                blockLeftDrawUntilRelease = true;
                if (!(s != std::nullopt && s == GridPos{row, col})) {
                    g = {row, col};
                    grid[row][col] = CellState::Empty;
                    state_change = true;
                }
            } else if (leftDown && !blockLeftDrawUntilRelease &&
                       s != GridPos{row, col} && g != GridPos{row, col} &&
                       grid[row][col] != CellState::Wall) {
                grid[row][col] = CellState::Wall;
                state_change = true;
            } else if (rightDown &&
                       (s == GridPos{row, col} || g == GridPos{row, col} ||
                        grid[row][col] == CellState::Wall)) {
                if (s != std::nullopt && s == GridPos{row, col}) {
                    s = std::nullopt;
                    sp = SearchPeriod::Unready;
                }
                if (g != std::nullopt && g == GridPos{row, col}) {
                    g = std::nullopt;
                    sp = SearchPeriod::Unready;
                }
                grid[row][col] = CellState::Empty;
                state_change = true;
            }
            if (state_change) {
                reset_all_state(gScore, cameFrom, openSet, closed, path, s, g);
                if (s != std::nullopt && g != std::nullopt) {
                    sp = SearchPeriod::Searching;
                }
            }
        }

        while (!openSet.empty() &&
               closed[openSet.top().pos.row][openSet.top().pos.col]) {
            openSet.pop();
        }
        if (sp == SearchPeriod::Searching && !openSet.empty() && searchTimer > searchInterval) {
            searchTimer = 0.f;
            auto p = openSet.top();
            openSet.pop();
            if (p.pos == g.value()) {
                sp = SearchPeriod::Found;
                openSet = {};
                GridPos cur = g.value();
                while (cur != s.value()) {
                    auto t = cameFrom[cur.row][cur.col].value();
                    path[t.row][t.col] = true;
                    cur = t;
                }
            } else {
                closed[p.pos.row][p.pos.col] = true;
                auto nb = get_neighbors(grid, p.pos);
                for (auto &grid_pos : nb) {
                    // 需要注意要累计之前的gScore
                    int g_score = gScore[p.pos.row][p.pos.col] +
                                  heuristic(grid_pos, p.pos);
                    int h_score = heuristic(grid_pos, g.value());
                    // 如果小于之前的可到达分数,更新下
                    if (g_score < gScore[grid_pos.row][grid_pos.col]) {
                        gScore[grid_pos.row][grid_pos.col] = g_score;
                        openSet.emplace(grid_pos, g_score + h_score, h_score);
                        cameFrom[grid_pos.row][grid_pos.col] = p.pos;
                    }
                }
            }
        }else if (sp == SearchPeriod::Searching && openSet.empty()) {
            sp = SearchPeriod::NoPath;
        }

        BeginDrawing();

        ClearBackground(BLACK);
        DrawText("Left: draw wall     Right: erase", OFFSET_X, 20, 20,
                 RAYWHITE);
        DrawText(showScores ? "[D] G scores: ON" : "[D] G scores: OFF", 650,
                 20, 20, showScores ? GREEN : LIGHTGRAY);

        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                Rectangle rect{static_cast<float>(OFFSET_X + col * CELL_SIZE),
                               static_cast<float>(OFFSET_Y + row * CELL_SIZE),
                               static_cast<float>(CELL_SIZE),
                               static_cast<float>(CELL_SIZE)};

                Color c = GetCellColor(grid[row][col]);
                Color scoreColor = BLACK;
                if (s != std::nullopt && row == s->row && col == s->col) {
                    c = GREEN;
                    scoreColor = RAYWHITE;
                } else if (g != std::nullopt && row == g->row &&
                           col == g->col) {
                    c = RED;
                    scoreColor = RAYWHITE;
                } else if (path[row][col]) {
                    c = YELLOW;
                } else if (closed[row][col]) {
                    c = PURPLE;
                    scoreColor = RAYWHITE;
                }
                DrawRectangleRec(rect, c);
                if (showScores &&
                    gScore[row][col] != std::numeric_limits<int>::max()) {
                    constexpr int scoreFontSize = 20;
                    const char *scoreText = TextFormat("%d", gScore[row][col]);
                    const int scoreWidth =
                        MeasureText(scoreText, scoreFontSize);
                    DrawText(scoreText,
                             static_cast<int>(rect.x +
                                              (rect.width - scoreWidth) / 2),
                             static_cast<int>(rect.y +
                                              (rect.height - scoreFontSize) / 2),
                             scoreFontSize, scoreColor);
                }
                DrawRectangleLinesEx(rect, 1.0F, GRAY);
            }
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
