#include "board.h"
#include <raylib.h> // For LoadFileText, UnloadFileText
#include <iostream> // For std::cerr
#include <string>
#include <cmath> // For M_PI, sin, cos if doing precise pentagon geometry

// For pentagon layout. Angles in radians.
const float PI = 3.1415926535f;
const float PENTAGON_ANGLE = (2.0f * PI) / 5.0f;       // 72 degrees
const float PENTAGON_SIDE_ANGLE = (3.0f * PI) / 10.0f; // 54 degrees, angle from center to midpoint of a side base

Board::Board() : player_start_pos({0,0}), goal_pos({0,0}) {
    InitializePentagonLayout();
}

void Board::InitializePentagonLayout() {
    // This is for the visual representation primarily.
    // The logical grid is still rows/cols. Movement logic will map to this.
    // For "point up" pentagons in a row, then "point down" in next (interlocking):
    // Width of pentagon at widest point: 2 * radius * sin(PENTAGON_SIDE_ANGLE)
    // Height of pentagon: radius + radius * cos(PENTAGON_ANGLE/2) (from center to point + center to flat base midpoint)
    // Let's use simpler dx, dy for drawing for now as in original code, will refine for pentagonal movement

    pentagon_col_width = pentagon_radius * 2 * sin(PI * 2.0f/5.0f * 0.5f) * 2 * 0.8f ; // Approx width
    pentagon_row_height = pentagon_radius * (1.0f + cos(PI/5.0f)) * 0.75f; // Approx height between rows center
    pentagon_row_offset = pentagon_col_width / 2.0f;


    // Values from original main.cpp, adjust as necessary
    pentagon_dx_spacing = pentagon_radius * 1.6f; // Visual spacing
    pentagon_dy_spacing = pentagon_radius * 1.5f; // Visual spacing

}


bool Board::LoadMazeFromFile(const std::string& filename)
{
    char *file_text_data = LoadFileText(filename.c_str());
    if (file_text_data == nullptr)
    {
        std::cerr << "Error: [Board] Failed to load maze file: " << filename << std::endl;
        // Initialize with a default tiny empty maze to prevent crashes if GetGrid is called
        grid.assign(1, std::vector<int>(1, 0)); // 1x1 path cell
        player_start_pos = {0,0};
        goal_pos = {0,0};
        return false;
    }
    std::string file_content(file_text_data);
    UnloadFileText(file_text_data);

    bool success = ParseMazeFile(file_content);
    if (!success) {
        grid.assign(1, std::vector<int>(1, 0)); // Fallback
        player_start_pos = {0,0};
        goal_pos = {0,0};
        return false;
    }

    // After parsing, set goal position (conventionally bottom-right)
    if (!grid.empty() && !grid[0].empty()) {
        goal_pos = { (int)grid.size() - 1, (int)grid[0].size() - 1 };
    } else { // Should not happen if ParseMazeFile ensures grid is not empty
        goal_pos = {0,0};
    }

    // Player start position could be fixed (0,0) or read from map ('S' char)
    // For now, assuming (0,0) unless map parsing sets it differently.
    // If grid[0][0] is wall, Game class should find a new start.
    player_start_pos = {0,0}; // Default, can be overridden by ParseMazeFile if it finds 'S'

    InitializePentagonLayout(); // Recalculate layout params based on loaded maze if needed (e.g. adapt radius)
    return true;
}

bool Board::ParseMazeFile(const std::string& file_content)
{
    grid.clear();
    initial_dynamic_walls.clear();
    std::vector<std::string> lines;
    std::string current_line;

    for (char ch : file_content)
    {
        if (ch == '\n')
        {
            if (!current_line.empty()) lines.push_back(current_line);
            current_line.clear();
        }
        else if (ch != '\r') // Ignore carriage return
        {
            current_line += ch;
        }
    }
    if (!current_line.empty()) lines.push_back(current_line);

    if (lines.empty())
    {
        std::cerr << "Error: [Board] Maze file is empty or has invalid format." << std::endl;
        return false;
    }

    // Determine dimensions from the longest line for columns
    // and number of lines for rows.
    int num_rows = lines.size();
    int num_cols = 0;
    for(const auto& line : lines) {
        int current_line_cols = 0;
        for(char c : line) if (c != ' ') current_line_cols++; // Count non-space characters
        if (current_line_cols > num_cols) num_cols = current_line_cols;
    }


    if (num_rows == 0 || num_cols == 0) {
        std::cerr << "Error: [Board] Maze has zero rows or columns." << std::endl;
        return false;
    }

    grid.resize(num_rows, std::vector<int>(num_cols));

    for (int r = 0; r < num_rows; ++r)
    {
        int c_grid = 0; // Current column in the grid, skipping spaces
        for (char cell_char : lines[r])
        {
            if (cell_char == ' ') continue; // Skip spaces
            if (c_grid >= num_cols) break; // Prevent writing past allocated grid columns

            if (cell_char >= '0' && cell_char <= '3')
            {
                grid[r][c_grid] = cell_char - '0';
                if (grid[r][c_grid] == 3) // Dynamic wall type
                {
                    // Default properties for dynamic walls, can be extended if map format supports it
                    initial_dynamic_walls.push_back({r, c_grid, 3, 3}); // Default 3 turns
                }
            }
            // Add support for 'S' (start) and 'E' (end) if desired
            else if (cell_char == 'S') {
                grid[r][c_grid] = 0; // Path
                player_start_pos = {r, c_grid};
            } else if (cell_char == 'E') {
                grid[r][c_grid] = 0; // Path
                goal_pos = {r, c_grid};
            }
            else
            {
                grid[r][c_grid] = 0; // Default to path for unknown characters
            }
            c_grid++;
        }
         // If a line was shorter than num_cols, fill remaining with walls or paths
        while(c_grid < num_cols) {
            grid[r][c_grid] = 1; // Default to wall if line is short
            c_grid++;
        }
    }
    return true;
}

const MazeGrid& Board::GetGrid() const
{
    return grid;
}

int Board::GetRows() const
{
    return grid.empty() ? 0 : grid.size();
}

int Board::GetCols() const
{
    return grid.empty() ? 0 : grid[0].size();
}

int Board::GetCellType(int r, int c) const
{
    if (r < 0 || r >= GetRows() || c < 0 || c >= GetCols())
    {
        return 1; // Treat out-of-bounds as wall for safety
    }
    return grid[r][c];
}

// This function will need to be more sophisticated for pentagonal movement.
// For now, it's based on the rectangular grid interpretation.
bool Board::IsCellWalkable(int r, int c, int current_turn, const std::vector<DynamicWall>& dynamic_walls_state) const
{
    if (r < 0 || r >= GetRows() || c < 0 || c >= GetCols())
    {
        return false; // Out of bounds
    }

    int cell_type = grid[r][c];
    switch (cell_type)
    {
    case 0: return true;  // Path
    case 1: return false; // Wall
    case 2:               // Even/Odd turn wall
        return (current_turn % 2 == 0); // Walkable on even turns
    case 3:               // Dynamic wall
        for (const auto& dw : dynamic_walls_state) {
            if (dw.row == r && dw.col == c) {
                return (dw.current_turns <= 0); // Walkable if turns countdown reached zero
            }
        }
        // If not in dynamic_walls_state, it means it was never a dynamic wall or already opened permanently.
        // Assuming initial grid state has 3 for dynamic walls that are initially closed.
        // This logic might need refinement based on how dynamic walls are managed post-opening.
        // If an opened dynamic wall changes its type in the main grid to 0, then this case 3
        // would only be hit if it's still an active dynamic wall.
        return false; // Default to not walkable if it's type 3 but not found in active list (shouldn't happen if list is synced)
    default: return false; // Unknown cell type
    }
}

std::pair<int, int> Board::GetPlayerStart() const
{
    return player_start_pos;
}

std::pair<int, int> Board::GetGoalPosition() const
{
    // If not set by map, defaults to bottom-right
    if (GetRows() > 0 && GetCols() > 0 && (goal_pos.first == 0 && goal_pos.second == 0 && !(GetRows()==1 && GetCols()==1))) {
         return {GetRows() - 1, GetCols() - 1};
    }
    return goal_pos;
}

const std::vector<DynamicWall>& Board::GetDynamicWallsConfig() const
{
    return initial_dynamic_walls;
}

std::pair<float, float> Board::CalculateRenderOffsets(int screen_width, int ui_panel_width, int screen_height) const {
    if (GetRows() == 0 || GetCols() == 0) return {0.0f, 0.0f};

    // This calculation is for the "staggered rows" layout of pentagons.
    // Effective width: For N columns, it's roughly (Cols - 0.5 for staggered) * pentagon_col_width
    // Effective height: Rows * pentagon_row_height
    // This needs to be based on the actual drawing logic in Renderer.
    // Using the dx_spacing, dy_spacing for now (like original)
    float total_maze_width = (GetCols() - 1) * pentagon_dx_spacing + (2 * pentagon_radius); // Approximate width
    float total_maze_height = (GetRows() - 1) * pentagon_dy_spacing + (2 * pentagon_radius); // Approximate height

    float offset_x = (static_cast<float>(screen_width) - total_maze_width - ui_panel_width) / 2.0f + ui_panel_width;
    float offset_y = (static_cast<float>(screen_height) - total_maze_height) / 2.0f;

    return {offset_x < ui_panel_width ? ui_panel_width + 20.f : offset_x, offset_y < 0 ? 20.f : offset_y};
}

// Note: The original maze.h/maze.cpp had ShortestPathBfs. This will be moved to pathfinding.h/cpp.
// The MazeGrid typedef is now in board.h.
// The DynamicWall struct was in main, now better in game.h as it's part of game state.
// Board's responsibility is loading the static map structure and initial dynamic wall configs.
// Game class will manage the current state of dynamic walls.
// Pentagon drawing functions from pentagon.h/cpp will be used by Renderer.
// This Board class focuses on the grid data itself.
// The pentagonal movement logic will be a significant change affecting Board::IsCellWalkable (implicitly)
// and how player coordinates are updated in Game, and how BFS explores neighbors in Pathfinding.
