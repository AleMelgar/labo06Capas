#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include "raylib.h"
#include "board.h" // Game class needs to know about Board
#include "input_handler.h" // For PlayerAction enum

// Player struct remains mostly the same
struct Player
{
    int row, col;
    std::vector<std::pair<int, int>> movement_history;
    bool active;
};

// DynamicWall struct remains the same
struct DynamicWall
{
    int row, col;
    int initial_turns_to_open; // Store the original value
    int current_turns;         // Countdown
};

// GameState enum remains the same
enum class GameState
{
    PLAYING,
    GAME_OVER_WIN,
    GAME_OVER_LOSE,
    // PATH_DISPLAY // This state might not be needed if path is just an overlay
};

class Game
{
public:
    // Constructor now takes a const reference to a Board object
    Game(const Board& board_ref);
    ~Game();

    void Update();       // Main game logic update (turn, clone, walls, win/loss)
    // ProcessInput is removed from public, Game will call a private method if needed, or main loop passes actions
    void ProcessPlayerMove(PlayerAction action); // New method to handle player move based on action

    // Render method is removed, Renderer class handles all drawing.
    // Game provides data to Renderer through getters.

    // Getters for game state and properties needed by main loop or other classes
    GameState GetCurrentState() const;
    int GetCurrentTurn() const;
    const Player& GetPlayer() const; // For Renderer
    const Player& GetClone() const;   // For Renderer
    std::pair<int, int> GetPlayerPosition() const; // For Pathfinding
    const std::vector<DynamicWall>& GetDynamicWallStates() const; // For Pathfinding & Renderer

    bool HasPlayerMovedThisTurn() const;
    void ResetPlayerMovedFlag();
    void Restart(const Board& board_ref); // Method to reset the game

    // bool ShouldClose() const; // This is usually handled by main loop directly with Raylib's WindowShouldClose()

    // Maze related getters are now through the Board reference
    // int GetRows() const; // Get from board_ref
    // int GetCols() const; // Get from board_ref
    // int GetCellType(int r, int c) const; // Get from board_ref, considering dynamic states

private:
    void InitializeGameMembers(const Board& board_ref); // Helper to init/reset game members based on board
    void HandlePlayerMovement(int dr, int dc); // Existing movement validation and execution logic
    void UpdateClone();
    void UpdateDynamicWalls();
    void CheckEndConditions();
    void CheckCloneCollision(); // Specific check for clone catching player

    const Board& game_board; // Reference to the game board (maze layout, static properties)

    Player player;
    Player clone;
    std::vector<DynamicWall> live_dynamic_walls; // Current state of dynamic walls, initialized from game_board

    int current_turn;
    GameState current_game_state;
    bool player_moved_this_turn; // Flag for BFS update

    static constexpr int CLONE_ACTIVATION_TURNS = 6; // Or read from config/board

    // Colors and screen dimensions are better handled by Renderer or a Config module
    // Removing them from Game class to simplify its responsibility.
};

#endif // GAME_H
