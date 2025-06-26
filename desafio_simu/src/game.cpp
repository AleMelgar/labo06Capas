#include "game.h"
#include "config.h" // Para constantes globales
#include <iostream>  // Para std::cerr

Game::Game(const std::string& maze_filename)
    : player(0, 0), // Posición inicial por defecto, se ajustará en InitializeGame
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
    // El destructor de Renderer se encargará de CloseWindow()
}

void Game::InitializeGame(const std::string& maze_filename)
{
    if (!grid_manager.LoadMaze(maze_filename))
    {
        std::cerr << "Error crítico: No se pudo cargar el laberinto '" << maze_filename << "'." << std::endl;
        // Podríamos establecer un estado de error o salir, pero por ahora el juego podría no funcionar.
        // Validar en GridManager si el laberinto es realmente jugable (ej. no vacío)
        current_game_state = GameState::GAME_OVER; // O un nuevo estado ERROR
        return;
    }

    // Establecer posición inicial del jugador (ej. 0,0 o desde el archivo de laberinto si se define)
    // Por ahora, asumimos que el jugador siempre empieza en (0,0) si es caminable.
    // La lógica original tenía una comprobación: if (maze[player.row][player.col] != 0) error.
    // Esto ahora sería:
    if (!grid_manager.IsCellWalkable(player.GetRow(), player.GetCol(), 0, true, clone.GetPos(), clone.IsActive())) {
        std::cerr << "Error: La posición inicial del jugador (0,0) no es transitable." << std::endl;
        // Buscar una celda inicial alternativa o marcar error.
        // Por ahora, para simplificar, si (0,0) no es válida, el juego no será jugable.
        // Intentemos buscar la primera celda abierta:
        bool found_start = false;
        for (int r = 0; r < grid_manager.GetRows(); ++r) {
            for (int c = 0; c < grid_manager.GetCols(); ++c) {
                if (grid_manager.IsCellWalkable(r, c, 0, true, clone.GetPos(), clone.IsActive())) {
                    player.Move(r,c);
                    found_start = true;
                    break;
                }
            }
            if (found_start) break;
        }
        if (!found_start) {
             std::cerr << "Error: No se encontró una posición inicial válida para el jugador." << std::endl;
            current_game_state = GameState::GAME_OVER; // O un estado de error.
            return;
        }
    }
    
    player.RecordMovement(); // Grabar posición inicial

    CalculateMazeRenderOffsets();
    current_turn = 0; // Asegurar que el turno es 0 al inicio
    clone.SetActive(false); // Clon inactivo al inicio
    current_game_state = GameState::PLAYING;
}

void Game::CalculateMazeRenderOffsets()
{
    if (grid_manager.GetRows() == 0 || grid_manager.GetCols() == 0) return;

    float total_maze_width = (grid_manager.GetCols() - 1) * PENTAGON_DX + PENTAGON_DX;
    float total_maze_height = (grid_manager.GetRows() - 1) * PENTAGON_DY + PENTAGON_DY;

    // El panel UI ocupa 230px a la izquierda
    float drawable_screen_width = SCREEN_WIDTH - 230;
    
    maze_render_start_x = 230 + (drawable_screen_width - total_maze_width) / 2.0f;
    maze_render_start_y = (SCREEN_HEIGHT - total_maze_height) / 2.0f;
}


void Game::Run()
{
    while (!WindowShouldClose() && current_game_state != GameState::GAME_OVER) // Permitir salir de GAME_OVER si se implementa reinicio
    {
        ProcessInput();
        if (current_game_state == GameState::PLAYING) { // Solo actualizar si se está jugando
            Update();
        }
        Render();
    }
     // Bucle post-juego para mostrar "GAME OVER" hasta que se cierre la ventana
    while (!WindowShouldClose() && current_game_state == GameState::GAME_OVER) {
        Render(); // Solo renderizar
         if (IsKeyPressed(KEY_R)) { // Ejemplo de reinicio
            InitializeGame("maze.txt"); // O el último laberinto usado
        }
    }
}

void Game::ProcessInput()
{
    if (current_game_state == GameState::PLAYING)
    {
        int next_player_row = player.GetRow();
        int next_player_col = player.GetCol();
        bool player_attempted_move = false;

        if (IsKeyPressed(KEY_UP))    { next_player_row--; player_attempted_move = true; }
        if (IsKeyPressed(KEY_DOWN))  { next_player_row++; player_attempted_move = true; }
        if (IsKeyPressed(KEY_LEFT))  { next_player_col--; player_attempted_move = true; }
        if (IsKeyPressed(KEY_RIGHT)) { next_player_col++; player_attempted_move = true; }

        if (player_attempted_move)
        {
            if (grid_manager.IsCellWalkable(next_player_row, next_player_col, current_turn + 1, true, clone.GetPos(), clone.IsActive()))
            {
                player.Move(next_player_row, next_player_col);
                player.RecordMovement();
                current_turn++; // El turno avanza solo si el jugador se mueve

                // Lógica de actualización post-movimiento del jugador
                grid_manager.UpdateDynamicWalls(current_turn);

                // Activar y mover clon
                if (!clone.IsActive() && current_turn >= CLONE_ACTIVATION_TURNS) {
                    clone.SetActive(true);
                    // El clon toma la posición inicial del jugador (movimiento 0)
                    if (!player.GetMovementHistory().empty()) {
                         // El clon.UpdatePositionFromHistory se encargará de esto basado en el turno.
                         // Pero necesitamos asegurarnos que la primera vez que se active, tome la pos[0]
                         // Si player_history[0] es la pos DESPUES del primer movimiento, y el clon debe estar en la pos inicial del jugador
                         // entonces el clon debe tomar player_history[0] en el turno CLONE_ACTIVATION_TURNS.
                         // La lógica de UpdatePositionFromHistory ya debería manejar esto.
                    }
                }

                if (clone.IsActive()) {
                    clone.UpdatePositionFromHistory(player.GetMovementHistory(), current_turn);
                    // Comprobar colisión jugador-clon DESPUÉS de que ambos se muevan
                    if (player.GetPos() == clone.GetPos()) {
                        // El jugador se movió a la celda donde el clon acaba de aparecer.
                        // Esto no debería pasar si IsCellWalkable del jugador lo previene.
                        // Pero si el clon se mueve a la celda del jugador, es game over.
                        // La lógica original: if (clone_active && next_row == clone.row && next_col == clone.col) cellIsBlocked = true;
                        // Esto previene que el JUGADOR se mueva a la celda del clon.
                        // ¿Qué pasa si el CLON se mueve a la celda del jugador?
                        // "El jugador y el clon no pueden ocupar la misma celda al mismo tiempo."
                        // Si después del movimiento del clon, P.pos == C.pos, entonces es game over.
                         current_game_state = GameState::GAME_OVER; // O manejarlo de otra forma
                         std::cout << "Game Over: Colisión con el clon en el turno " << current_turn << std::endl;
                         return; // Salir de ProcessInput para no seguir
                    }
                }

                // Comprobar condición de victoria
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
        InitializeGame("maze.txt"); // O el último laberinto usado
    }
}

void Game::Update()
{
    // La lógica de actualización principal ahora ocurre en ProcessInput después de un movimiento válido.
    // Esto incluye:
    // - Avance de turno
    // - Actualización de paredes dinámicas
    // - Activación y movimiento del clon
    // - Comprobación de colisiones y victoria

    // Si hubiera otras lógicas de actualización independientes del input del jugador, irían aquí.
    // Por ejemplo, animaciones, efectos de partículas, etc.
    // En este juego, el "tiempo" (turnos) solo avanza cuando el jugador se mueve.
}

void Game::Render()
{
    renderer.BeginDrawingSequence();

    renderer.DrawGrid(grid_manager, current_turn, maze_render_start_x, maze_render_start_y);
    
    renderer.DrawEntity(player, maze_render_start_x, maze_render_start_y, 0.8f, PLAYER_GREEN, "TÚ");
    
    if (clone.IsActive()) {
        renderer.DrawEntity(clone, maze_render_start_x, maze_render_start_y, 0.6f, CLONE_BLUE, "CLON");
    }

    if (show_path_solution) {
        // Calcular el camino solo cuando se necesita o si el grid cambia.
        // Por ahora, lo calculamos cada vez que se muestra para simplicidad,
        // pero podría optimizarse.
        std::vector<Position> shortest_path = grid_manager.CalculateShortestPath(
            player.GetPos(),
            {grid_manager.GetRows() - 1, grid_manager.GetCols() - 1},
            current_turn // El cálculo del camino debe considerar el estado actual del juego
        );
        if (!shortest_path.empty()) {
            renderer.DrawPath(shortest_path, maze_render_start_x, maze_render_start_y, 0.65f, PATH_SOLUTION_COLOR);
        }
    }

    renderer.DrawUI(current_turn, current_game_state);

    renderer.EndDrawingSequence();
}
