#include "raylib.h"
#include "game.h"       // Manages game logic, state, player, clone
#include "board.h"      // Manages maze structure, loading, properties
#include "renderer.h"   // Manages all drawing
#include "input_handler.h" // Manages keyboard input
#include "pathfinding.h" // For FindShortestPathBFS
#include <iostream>      // For std::cerr
#include <string>        // For std::string

// Screen dimensions
constexpr int SCREEN_WIDTH = 900;
constexpr int SCREEN_HEIGHT = 600;
const char* WINDOW_TITLE = "Escape the Grid - Modular";

// Maze file
const std::string MAZE_FILE = "maze.txt"; // Make sure this file exists in the correct path relative to executable

int main()
{
    // Initialization of Window and Raylib
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60); // Set desired frame rate

    // --- Game Object Creation ---
    Board game_board; // Create the board object
    if (!game_board.LoadMazeFromFile(MAZE_FILE)) {
        TraceLog(LOG_ERROR, "Failed to load maze: %s. Ensure it's in the execution directory.", MAZE_FILE.c_str());
        CloseWindow();
        return 1; // Exit if maze loading fails
    }

    // Game class instance, now takes the already loaded board.
    Game escape_the_grid_game(game_board);

    InputHandler input_handler; // Handles all keyboard inputs

    // Renderer needs screen dimensions and a reference to the board for layout.
    Renderer game_renderer(SCREEN_WIDTH, SCREEN_HEIGHT, game_board);

    // --- Game State Variables ---
    bool show_path_solution = false; // Toggled by 'S' key
    std::vector<std::pair<int, int>> current_bfs_path; // Stores the path from BFS

    // --- Main Game Loop ---
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // 1. PROCESS INPUT
        input_handler.Update(); // Read current input state

        if (input_handler.IsTogglePathPressed()) {
            show_path_solution = !show_path_solution;
        }

        if (escape_the_grid_game.GetCurrentState() == GameState::PLAYING) {
            PlayerAction move_action = input_handler.GetMoveAction();
            if (move_action != PlayerAction::NONE) {
                escape_the_grid_game.ProcessPlayerMove(move_action);
            }
        } else { // If game is over (win/lose)
            if (input_handler.IsRestartPressed()) {
                if (game_board.LoadMazeFromFile(MAZE_FILE)){
                    escape_the_grid_game.Restart(game_board);
                    show_path_solution = false;
                    current_bfs_path.clear();
                } else {
                     TraceLog(LOG_ERROR, "Failed to reload maze for restart: %s.", MAZE_FILE.c_str());
                }
            }
        }

        // 2. UPDATE GAME LOGIC
        escape_the_grid_game.Update();

        // Dynamic BFS: Recalculate path
        if (escape_the_grid_game.GetCurrentState() == GameState::PLAYING) {
            if (show_path_solution && (escape_the_grid_game.HasPlayerMovedThisTurn() || current_bfs_path.empty())) {
                current_bfs_path = Pathfinding::FindShortestPathBFS(
                    game_board,
                    escape_the_grid_game.GetPlayerPosition(),
                    game_board.GetGoalPosition(),
                    escape_the_grid_game.GetDynamicWallStates(),
                    escape_the_grid_game.GetCurrentTurn()
                );
                escape_the_grid_game.ResetPlayerMovedFlag();
            } else if (!show_path_solution && !current_bfs_path.empty()) {
                current_bfs_path.clear(); // Clear path if 'S' is toggled off
            }
        } else if (!current_bfs_path.empty() && escape_the_grid_game.GetCurrentState() != GameState::GAME_OVER_WIN && escape_the_grid_game.GetCurrentState() != GameState::GAME_OVER_LOSE) {
            // Clear path if game not playing (e.g. menu state) unless it's game over and we want to show final path
            // current_bfs_path.clear();
        }


        // 3. RENDER OUTPUT
        game_renderer.DrawGame(
            game_board,
            escape_the_grid_game.GetPlayer(),
            escape_the_grid_game.GetClone(),
            escape_the_grid_game.GetDynamicWallStates(),
            escape_the_grid_game.GetCurrentTurn(),
            escape_the_grid_game.GetCurrentState(),
            current_bfs_path,
            show_path_solution
        );
    }

    // De-Initialization
    CloseWindow();

    return 0;
}
