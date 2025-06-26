#pragma once

#include "config.h"
#include "grid_manager.h"
#include "player_entity.h"
#include "renderer.h"
#include <string>
#include <vector>

class Game
{
public:
    Game(const std::string& maze_filename);
    ~Game();

    void Run();

private:
    GridManager grid_manager;
    Player player;
    Clone clone;
    Renderer renderer;
    
    int current_turn;
    GameState current_game_state;
    bool show_path_solution;

    // Variables para el layout del grid
    float maze_render_start_x;
    float maze_render_start_y;

    void InitializeGame(const std::string& maze_filename);
    void Update();
    void ProcessInput();
    void Render();
    
    void CalculateMazeRenderOffsets();
};
