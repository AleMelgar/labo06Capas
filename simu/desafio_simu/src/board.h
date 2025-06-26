#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>
#include "game.h" // For DynamicWall, if board is responsible for initializing them.
                  // Or forward declare DynamicWall if game.h includes board.h to avoid circular dependency.
                  // For simplicity now, including game.h

// Define MazeGrid as a type alias for clarity
using MazeGrid = std::vector<std::vector<int>>;

class Board
{
public:
    Board();
    //~Board(); // Destructor if needed for dynamic allocations

    bool LoadMazeFromFile(const std::string& filename);

    const MazeGrid& GetGrid() const;
    int GetRows() const;
    int GetCols() const;
    int GetCellType(int r, int c) const; // Gets type of cell (wall, path, special)
    bool IsCellWalkable(int r, int c, int current_turn, const std::vector<DynamicWall>& dynamic_walls) const; // Considers dynamic states

    std::pair<int, int> GetPlayerStart() const; // Could be fixed or read from map file
    std::pair<int, int> GetGoalPosition() const;  // Could be fixed or read from map file

    // Dynamic walls initialization might be better here if map format supports defining their properties
    const std::vector<DynamicWall>& GetDynamicWallsConfig() const;


    // Pentagonal grid specifics - drawing parameters
    float GetPentagonRadius() const { return pentagon_radius; }
    float GetPentagonDx() const { return pentagon_dx; } // Horizontal distance between centers
    float GetPentagonDy() const { return pentagon_dy; } // Vertical distance between rows

    // Calculates render offsets based on screen size and maze dimensions
    std::pair<float, float> CalculateRenderOffsets(int screen_width, int ui_panel_width, int screen_height) const;


private:
    MazeGrid grid;
    std::vector<DynamicWall> initial_dynamic_walls; // Stores the initial configuration of dynamic walls
    std::pair<int, int> player_start_pos;
    std::pair<int, int> goal_pos;

    // Default properties for pentagonal grid rendering (can be configurable)
    float pentagon_radius = 30.0f; // Adjusted for potentially more cells
    // For a grid where pentagons interlock (point up, point down rows)
    // Width of pentagon: 2 * R * sin(54 degrees) approx R * 1.618
    // Height of pentagon: R * (1 + cos(36 degrees)) approx R * 1.809
    // Effective horizontal spacing (dx) can be tricky.
    // If side by side: dx = 2 * R * sin(54)
    // If rows are offset: dx needs to account for this for neighbor finding.
    // For drawing, dx might be width of one pentagon. For movement, it's more complex.
    // Let's assume a rectangular grid layout for now, and pentagon drawing is laid over it.
    // The "5 directions" movement will be key.

    // These are for a simple rectangular underlying grid for pentagon centers
    float pentagon_dx_spacing; // How far apart horizontally centers are.
    float pentagon_dy_spacing; // How far apart vertically centers are.

    // These will be set based on the pentagon_radius and the chosen layout strategy
    float pentagon_col_width;  // Effective width a column of pentagons takes
    float pentagon_row_height; // Effective height a row of pentagons takes
    float pentagon_row_offset; // Horizontal offset for alternating rows in staggered layout

    void InitializePentagonLayout();


    // Helper to parse the maze file (similar to what was in main.cpp or game.cpp)
    // This will also identify dynamic walls and set them up in initial_dynamic_walls
    bool ParseMazeFile(const std::string& file_content);
};

#endif // BOARD_H
