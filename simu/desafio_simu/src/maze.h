#pragma once
#include <vector>

struct DynamicWall {
    int row, col, turns_to_open;
};

using MazeGrid = std::vector<std::vector<int>>;

MazeGrid LoadMazeFromFile(const char* filename);

// Solo necesaria si sigues usando esta versión simple en algún lugar
bool IsCellValid(int x, int y, const MazeGrid& grid, const std::vector<std::vector<bool>>& visited, int current_turn);

// Versión mejorada del BFS que toma en cuenta turnos y paredes dinámicas
std::vector<std::pair<int, int>> ShortestPathBfs(
    const MazeGrid& grid,
    std::pair<int, int> start,
    std::pair<int, int> end,
    const std::vector<DynamicWall>& dynamic_walls,
    int current_turn
);
