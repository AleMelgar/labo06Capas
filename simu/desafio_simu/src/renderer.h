#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include <vector>
#include <string>
#include "board.h" // Needs to know about Board to get dimensions and cell types
#include "game.h"  // Needs to know about Player, GameState, DynamicWall states

// Forward declare Game and Board if game.h / board.h include renderer.h to avoid circular dependency
// class Game;
// class Board;

class Renderer
{
public:
    Renderer(int screen_width, int screen_height, const Board& board_ref); // Takes const ref to board
    // If Game class holds all colors, pass Game const ref too, or pass colors struct
    // For now, renderer defines its own colors or gets them from Game object passed in render methods

    voidSetBoard(const Board* board); // To update board reference if needed, or pass board to draw methods

    void DrawGame(
        const Board& current_board,
        const Player& player,
        const Player& clone,
        const std::vector<DynamicWall>& dynamic_walls,
        int current_turn,
        GameState game_state,
        const std::vector<std::pair<int, int>>& path_to_display, // Path from BFS
        bool show_path_flag
    );

private:
    void DrawBoard(const Board& board_to_draw, const std::vector<DynamicWall>& dynamic_wall_states, int turn);
    void DrawEntities(const Board& board_to_draw, const Player& p, const Player& c);
    void DrawUI(const Board& board_to_draw, int turn, GameState state); // Pass Board for goal display if needed
    void DrawPath(const Board& board_to_draw, const std::vector<std::pair<int, int>>& path); // For BFS path

    int screen_width;
    int screen_height;
    const Board& board; // Store a reference to the board for its properties

    // Maze drawing properties, initialized from board
    float pentagon_radius;
    float pentagon_dx; // Horizontal distance between centers for drawing
    float pentagon_dy; // Vertical distance between row centers for drawing
    float render_offset_x;
    float render_offset_y;

    static constexpr int UI_PANEL_WIDTH = 230;

    // Colors (can be moved to a config struct/file)
    Color color_bg = {30, 35, 40, 255};
    Color color_ui_panel = {45, 50, 60, 255};
    Color color_text_ui_title = {102, 204, 255, 255};
    Color color_text_ui_info = {240, 240, 240, 255};
    Color color_text_ui_subtle = {180, 180, 180, 255};
    Color color_player = {0, 200, 120, 255};
    Color color_clone = {90, 120, 250, 255};
    Color color_wall_static = {100, 100, 100, 255};
    Color color_wall_dynamic_closed = {60, 60, 60, 255};
    Color color_cell_path = RAYWHITE;
    Color color_cell_even_turn_special = {70, 140, 255, 255}; // For type 2 cells on even turns
    Color color_cell_odd_turn_special = {255, 210, 50, 255};  // For type 2 cells on odd turns
    Color color_path_solution = {255, 0, 255, 150}; // Magenta for BFS path
    Color color_win_text = {0, 255, 120, 255};
    Color color_lose_text = {255, 80, 80, 255};


    // Entity drawing scales
    float player_radius_factor = 0.8f;
    float clone_radius_factor = 0.6f;
    float path_radius_factor = 0.4f; // For drawing BFS path nodes
};

#endif // RENDERER_H
