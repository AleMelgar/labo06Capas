#include "maze.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>
#include <algorithm>
#include <set>
#include <tuple>


// Loads a maze from a txt file and returns it as a mazeGrid (vector<vector<int>>))
MazeGrid LoadMazeFromFile(const char* filename) {
    MazeGrid maze;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        return maze;
    }

    std::string line;

    while (std::getline(file, line)) {
        std::cout << "Leyendo línea: " << line << std::endl; // Debug
        std::istringstream iss(line);
        std::vector<int> row;
        int val;
        while (iss >> val) row.push_back(val);
        if (!row.empty()) maze.push_back(row);
    }

    std::cout << "Filas leídas: " << maze.size() << std::endl; // Debug
    return maze;
}

bool IsCellValid(int x, int y, const MazeGrid& grid, const std::vector<std::vector<bool>>& visited, int current_turn) {
    if (x < 0 || y < 0 || x >= grid.size() || y >= grid[0].size()) return false;
    if (visited[x][y]) return false;

    int cell = grid[x][y];
    if (cell == 1) return false;
    if (cell == 2 && current_turn % 2 != 0) return false;

    return true;
}

bool IsCellOpen(int x, int y, const MazeGrid &maze, const std::vector<std::vector<bool>> &visited,
                const std::vector<DynamicWall> &dynamic_walls, int turn) {
    int rows = maze.size(), cols = maze[0].size();
    if (x < 0 || y < 0 || x >= rows || y >= cols) return false;
    if (visited[x][y]) return false;

    int cell = maze[x][y];

    if (cell == 1) return false;
    if (cell == 2 && turn % 2 != 0) return false;
    if (cell == 3) {
        for (const auto &dw : dynamic_walls)
            if (dw.row == x && dw.col == y && dw.turns_to_open > turn)
                return false;
    }

    return true;
}


std::vector<std::pair<int, int>> ShortestPathBfs(
    const MazeGrid &maze,
    std::pair<int, int> start,
    std::pair<int, int> end,
    const std::vector<DynamicWall> &dynamic_walls,
    int current_turn
) {
    int rows = maze.size();
    int cols = maze[0].size();

    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<std::pair<int, int>>> predecessor(rows, std::vector<std::pair<int, int>>(cols, {-1, -1}));

    std::queue<std::tuple<int, int, int>> q; // x, y, turno
    q.push({start.first, start.second, current_turn});
    visited[start.first][start.second] = true;

    const int dx[] = {-1, 1, 0, 0};
    const int dy[] = {0, 0, -1, 1};

    while (!q.empty()) {
        auto [x, y, turn] = q.front(); q.pop();

        if (std::make_pair(x, y) == end)
            break;

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            int next_turn = turn + 1;

            if (IsCellOpen(nx, ny, maze, visited, dynamic_walls, next_turn)) {
                visited[nx][ny] = true;
                predecessor[nx][ny] = {x, y};
                q.push({nx, ny, next_turn});
            }
        }
    }

    std::vector<std::pair<int, int>> path;
    if (!visited[end.first][end.second]) return path;

    std::pair<int, int> current = end;
    while (current != start) {
        path.push_back(current);
        current = predecessor[current.first][current.second];
    }

    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}
