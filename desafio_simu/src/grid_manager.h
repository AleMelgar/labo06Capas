#pragma once

#include "config.h" // Para MazeGrid, DynamicWall, CellType, Position
#include <vector>
#include <string>
#include <utility> // Para std::pair

class GridManager
{
public:
    MazeGrid grid_data; // Los datos crudos del laberinto (0, 1, 2, 3)
    std::vector<DynamicWall> dynamic_walls_list;
    int num_rows;
    int num_cols;

    GridManager();
    ~GridManager() = default;

    bool LoadMaze(const std::string &filename);

    int GetCellType(int r, int c) const;
    bool IsCellWalkable(int r, int c, int current_turn, bool is_player = true, Position clone_pos = {-1, -1}, bool clone_is_active = false) const;

    void UpdateDynamicWalls(int current_turn); // Lógica para que las paredes dinámicas se abran

    int GetRows() const { return num_rows; }
    int GetCols() const { return num_cols; }

    std::vector<Position> CalculateShortestPath(Position start, Position end, int current_turn_for_calc) const;

private:

    bool IsCellOpenForPathfinding(
        int r, int c,
        const MazeGrid &temp_grid_state,
        const std::vector<std::vector<bool>> &visited,
        const std::vector<DynamicWall> &current_dynamic_walls_state,
        int turn_in_path,
        int current_turn_for_calc) const;
    
};
