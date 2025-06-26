#include "game.h"
#include "config.h"
#include <iostream>

Game::Game(const std::string& maze_filename)
    : player(0, 0),
      clone(-1, -1),
      renderer(SCREEN_WIDTH, SCREEN_HEIGHT, "Escape the Grid - Modular"),
      current_turn(0),
      current_game_state(GameState::PLAYING),
      show_path_solution(false),
      maze_render_start_x(0.0f),
      maze_render_start_y(0.0f)
{
    InitializeGame(maze_filename);
}

Game::~Game()
{
}

void Game::InitializeGame(const std::string& maze_filename)
{
    if (!grid_manager.LoadMaze(maze_filename))
    {
        std::cerr << "Error crítico: No se pudo cargar el laberinto '" << maze_filename << "'." << std::endl;
        current_game_state = GameState::GAME_OVER; 
        return;
    }

    bool found_start = false;
    for (int r = 0; r < grid_manager.GetRows(); ++r) {
        for (int c = 0; c < grid_manager.GetCols(); ++c) {
            if (grid_manager.GetCellType(r,c) == static_cast<int>(CellType::PATH) || 
                (grid_manager.GetCellType(r,c) == static_cast<int>(CellType::ALTERNATING_WALL) && (0 % 2 == 0)) ) {
                player.Move(r,c);
                found_start = true;
                break;
            }
        }
        if (found_start) break;
    }

    if (!found_start) {
         std::cerr << "Error: No se encontró una posición inicial válida para el jugador." << std::endl;
        current_game_state = GameState::GAME_OVER; 
        return;
    }
    
    player.RecordMovement(); 

    CalculateMazeRenderOffsets();
    current_turn = 0; 
    clone.SetActive(false); 
    current_game_state = GameState::PLAYING;
}

void Game::CalculateMazeRenderOffsets()
{
    if (grid_manager.GetRows() == 0 || grid_manager.GetCols() == 0) return;

    float total_maze_width = (grid_manager.GetCols() - 0.5f) * PENTAGON_DX;
    float total_maze_height = (grid_manager.GetRows() -1 ) * PENTAGON_DY + PENTAGON_RADIUS * 2;

    float drawable_screen_width = SCREEN_WIDTH - 230;
    
    maze_render_start_x = 230 + (drawable_screen_width - total_maze_width) / 2.0f;
    if (total_maze_width > drawable_screen_width) maze_render_start_x = 230 + PENTAGON_RADIUS;

    maze_render_start_y = (SCREEN_HEIGHT - total_maze_height) / 2.0f;
    if (total_maze_height > SCREEN_HEIGHT) maze_render_start_y = PENTAGON_RADIUS;
}

void Game::Run()
{
    while (!WindowShouldClose() && current_game_state != GameState::GAME_OVER) 
    {
        ProcessInput();
        if (current_game_state == GameState::PLAYING) { 
            Update();
        }
        Render();
    }
    while (!WindowShouldClose() && current_game_state == GameState::GAME_OVER) {
        Render(); 
         if (IsKeyPressed(KEY_R)) { 
            InitializeGame("maze.txt"); 
        }
    }
}

void Game::ProcessInput()
{
    if (current_game_state == GameState::PLAYING)
    {
        int current_player_row = player.GetRow();
        int current_player_col = player.GetCol();
        int next_player_row = current_player_row;
        int next_player_col = current_player_col;
        bool player_attempted_move = false;

        if (current_player_row % 2 == 0) { // Fila PAR (pentágono punta arriba)
            if (IsKeyPressed(KEY_Q)) { // Arriba-Izquierda (NW)
                next_player_row--;
                next_player_col--;
                player_attempted_move = true;
            } else if (IsKeyPressed(KEY_W)) { // Arriba-Derecha (NE)
                next_player_row--;
                player_attempted_move = true;
            } else if (IsKeyPressed(KEY_A)) { // Izquierda (W)
                next_player_col--;
                player_attempted_move = true;
            } else if (IsKeyPressed(KEY_D)) { // Derecha (E)
                next_player_col++;
                player_attempted_move = true;
            } else if (IsKeyPressed(KEY_X)) { // Abajo (S)
                next_player_row++;
                player_attempted_move = true;
            }
        } else { // Fila IMPAR (pentágono punta abajo)
            if (IsKeyPressed(KEY_W)) { // Arriba (N)
                next_player_row--;
                player_attempted_move = true;
            } else if (IsKeyPressed(KEY_A)) { // Izquierda (W)
                next_player_col--;
                player_attempted_move = true;
            } else if (IsKeyPressed(KEY_D)) { // Derecha (E)
                next_player_col++;
                player_attempted_move = true;
            } else if (IsKeyPressed(KEY_Z)) { // Abajo-Izquierda (SW)
                next_player_row++;
                player_attempted_move = true;
            } else if (IsKeyPressed(KEY_C)) { // Abajo-Derecha (SE)
                next_player_row++;
                next_player_col++;
                player_attempted_move = true;
            }
        }

        if (player_attempted_move)
        {
            if (grid_manager.IsCellWalkable(next_player_row, next_player_col, current_turn + 1, true, clone.GetPos(), clone.IsActive()))
            {
                player.Move(next_player_row, next_player_col);
                player.RecordMovement();
                current_turn++; 

                grid_manager.UpdateDynamicWalls(current_turn);

                if (!clone.IsActive() && current_turn >= CLONE_ACTIVATION_TURNS) {
                    clone.SetActive(true);
                }

                if (clone.IsActive()) {
                    clone.UpdatePositionFromHistory(player.GetMovementHistory(), current_turn);
                    if (player.GetPos() == clone.GetPos()) {
                         current_game_state = GameState::GAME_OVER; 
                         std::cout << "Game Over: Colisión con el clon en el turno " << current_turn << std::endl;
                         return; 
                    }
                }

                if (player.GetRow() == grid_manager.GetRows() - 1 && player.GetCol() == grid_manager.GetCols() - 1) {
                    current_game_state = GameState::GAME_OVER;
                    std::cout << "¡Has escapado en el turno " << current_turn << "!" << std::endl;
                }
            }
        }
    }

    if (IsKeyPressed(KEY_S)) {
        show_path_solution = !show_path_solution;
    }
     if (current_game_state == GameState::GAME_OVER && IsKeyPressed(KEY_R)) {
        InitializeGame("maze.txt"); 
    }
}

void Game::Update()
{
}

void Game::Render()
{
    renderer.BeginDrawingSequence();

    renderer.DrawGrid(grid_manager, current_turn, maze_render_start_x, maze_render_start_y);
    
    renderer.DrawEntity(player, player.GetRow(), maze_render_start_x, maze_render_start_y, 0.8f, PLAYER_GREEN, "TÚ");
    
    if (clone.IsActive()) {
        renderer.DrawEntity(clone, clone.GetRow(), maze_render_start_x, maze_render_start_y, 0.6f, CLONE_BLUE, "CLON");
    }

    if (show_path_solution) {
        std::vector<Position> shortest_path = grid_manager.CalculateShortestPath(
            player.GetPos(),
            {grid_manager.GetRows() - 1, grid_manager.GetCols() - 1},
            current_turn 
        );
        if (!shortest_path.empty()) {
            renderer.DrawPath(shortest_path, maze_render_start_x, maze_render_start_y, 0.65f, PATH_SOLUTION_COLOR);
        }
    }

    renderer.DrawUI(current_turn, current_game_state);

    renderer.EndDrawingSequence();
}
