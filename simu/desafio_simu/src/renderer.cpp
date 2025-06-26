#include "renderer.h"
#include "pentagon.h" // For DrawPentagon
#include <string> // For std::to_string, if TextFormat isn't used exclusively

Renderer::Renderer(int scr_width, int scr_height, const Board& board_ref)
    : screen_width(scr_width), screen_height(scr_height), board(board_ref)
{
    // Get pentagon properties from the board
    pentagon_radius = board.GetPentagonRadius();

    // These are the visual spacings for drawing, not necessarily the logical grid connection points
    // which will be more complex for pentagonal movement.
    // The original main.cpp used these for drawing:
    pentagon_dx = board.GetPentagonRadius() * 1.6f; // Use board's value if it's more refined
    pentagon_dy = board.GetPentagonRadius() * 1.5f; // Use board's value

    std::pair<float, float> offsets = board.CalculateRenderOffsets(screen_width, UI_PANEL_WIDTH, screen_height);
    render_offset_x = offsets.first;
    render_offset_y = offsets.second;
}


void Renderer::DrawGame(
    const Board& current_board, // Pass current board state, might be same as this->board
    const Player& player,
    const Player& clone,
    const std::vector<DynamicWall>& dynamic_walls,
    int current_turn,
    GameState game_state,
    const std::vector<std::pair<int, int>>& path_to_display,
    bool show_path_flag)
{
    BeginDrawing();
    ClearBackground(color_bg);

    DrawUI(current_board, current_turn, game_state); // Draw UI panel first
    DrawBoard(current_board, dynamic_walls, current_turn);

    if (show_path_flag && !path_to_display.empty()) {
        DrawPath(current_board, path_to_display);
    }

    DrawEntities(current_board, player, clone); // Draw player and clone on top

    // Game Over messages on top of everything else if game ended
    if (game_state == GameState::GAME_OVER_WIN) {
        const char* winMsg = "¡HAS ESCAPADO!";
        int fontSize = 30;
        int textWidth = MeasureText(winMsg, fontSize);
        DrawText(winMsg, (screen_width - UI_PANEL_WIDTH - textWidth) / 2 + UI_PANEL_WIDTH, screen_height / 2 - fontSize / 2, fontSize, color_win_text);
    } else if (game_state == GameState::GAME_OVER_LOSE) {
        const char* loseMsg = "¡ATRAPADO!";
        int fontSize = 30;
        int textWidth = MeasureText(loseMsg, fontSize);
        DrawText(loseMsg, (screen_width - UI_PANEL_WIDTH - textWidth) / 2 + UI_PANEL_WIDTH, screen_height / 2 - fontSize / 2, fontSize, color_lose_text);
    }


    EndDrawing();
}

void Renderer::DrawBoard(const Board& board_to_draw, const std::vector<DynamicWall>& dynamic_wall_states, int turn)
{
    int rows = board_to_draw.GetRows();
    int cols = board_to_draw.GetCols();

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            // Calculate pentagon center position
            // This is for the original staggered grid drawing.
            // For true pentagonal grid, this might need adjustment based on neighbor logic.
            float x_center = render_offset_x + c * pentagon_dx + (r % 2) * (pentagon_dx / 2.0f);
            float y_center = render_offset_y + r * pentagon_dy;

            Color cell_color = color_cell_path; // Default to path color
            int cell_type = board_to_draw.GetCellType(r, c);

            switch (cell_type)
            {
            case 0: // Path
                cell_color = color_cell_path;
                break;
            case 1: // Static Wall
                cell_color = color_wall_static;
                break;
            case 2: // Even/Odd Turn Wall
                cell_color = (turn % 2 == 0) ? color_cell_even_turn_special : color_cell_odd_turn_special;
                break;
            case 3: // Dynamic Wall (potentially, check state)
                {
                    bool is_active_dynamic = false;
                    int turns_left = 0;
                    for (const auto& dw : dynamic_wall_states) {
                        if (dw.row == r && dw.col == c && dw.current_turns > 0) {
                            is_active_dynamic = true;
                            turns_left = dw.current_turns;
                            break;
                        }
                    }
                    if (is_active_dynamic) {
                        cell_color = color_wall_dynamic_closed;
                        // Draw turns countdown on the dynamic wall
                        DrawText(TextFormat("%d", turns_left), x_center - MeasureText(TextFormat("%d", turns_left), 15)/2, y_center - 7, 15, WHITE);
                    } else {
                        // If it was type 3 but not in active list, it means it has opened. Render as path.
                        cell_color = color_cell_path;
                    }
                }
                break;
            default:
                cell_color = PINK; // Unknown cell type
                break;
            }
            // Determine if pentagon points up or down based on row (or a fixed orientation)
            // For now, all point up, as in original DrawPentagon usage.
            // bool point_up = (r % 2 == 0); // Example: even rows point up, odd point down
            DrawPentagon({x_center, y_center}, pentagon_radius, cell_color);
        }
    }
}

void Renderer::DrawEntities(const Board& board_to_draw, const Player& p, const Player& c)
{
    // Draw Player
    if (p.active) // Should always be true for main player
    {
        float px_center = render_offset_x + p.col * pentagon_dx + (p.row % 2) * (pentagon_dx / 2.0f);
        float py_center = render_offset_y + p.row * pentagon_dy;
        DrawPentagon({px_center, py_center}, pentagon_radius * player_radius_factor, color_player);
        DrawText("TÚ", px_center - MeasureText("TÚ", 14)/2, py_center - pentagon_radius - 10, 14, color_player);
    }

    // Draw Clone
    if (c.active)
    {
        float cx_center = render_offset_x + c.col * pentagon_dx + (c.row % 2) * (pentagon_dx / 2.0f);
        float cy_center = render_offset_y + c.row * pentagon_dy;
        DrawPentagon({cx_center, cy_center}, pentagon_radius * clone_radius_factor, color_clone);
        DrawText("CLON", cx_center - MeasureText("CLON", 14)/2, cy_center - pentagon_radius * clone_radius_factor - 10, 14, color_clone);
    }
}

void Renderer::DrawUI(const Board& board_to_draw, int turn, GameState state)
{
    DrawRectangle(0, 0, UI_PANEL_WIDTH, screen_height, color_ui_panel);
    int posY = 20;
    DrawText("ESCAPE THE GRID", 20, posY, 22, color_text_ui_title);
    posY += 30;
    DrawText(TextFormat("Turno: %d", turn), 20, posY, 18, color_text_ui_info);
    posY += 30;

    DrawText("Controles:", 20, posY, 18, color_text_ui_info);
    posY += 25;
    DrawText("- Flechas: Mover", 20, posY, 16, color_text_ui_info);
    posY += 20;
    // Add text for new pentagonal controls later e.g. QWE AD
    DrawText("- S: Mostrar/Ocultar Camino", 20, posY, 16, color_text_ui_info);
    posY += 20;
    DrawText("- R: Reiniciar (not impl.)", 20, posY, 16, color_text_ui_info);
    posY += 30;

    std::pair<int,int> goal_pos = board_to_draw.GetGoalPosition();
    DrawText(TextFormat("Meta: (%d, %d)", goal_pos.first, goal_pos.second), 20, posY, 14, color_text_ui_subtle);
    posY += 20;
    DrawText("Evita al clon y las paredes.", 20, posY, 14, color_text_ui_subtle);
    posY += 20;
    DrawText("Paredes azules/amarillas:", 20, posY, 14, color_text_ui_subtle);
    posY += 15;
    DrawText("  Abren en turnos PAR/IMPAR.", 20, posY, 14, color_text_ui_subtle);
    posY += 20;
    DrawText("Paredes grises con número:", 20, posY, 14, color_text_ui_subtle);
    posY += 15;
    DrawText("  Abren tras N turnos.", 20, posY, 14, color_text_ui_subtle);


    // Game state messages in UI panel (alternative to screen center)
    posY = screen_height - 100; // Position towards the bottom of UI
    if (state == GameState::GAME_OVER_WIN) {
        DrawText("¡NIVEL COMPLETADO!", 20, posY, 20, color_win_text);
    } else if (state == GameState::GAME_OVER_LOSE) {
        DrawText("¡FIN DEL JUEGO!", 20, posY, 20, color_lose_text);
    }
}

void Renderer::DrawPath(const Board& board_to_draw, const std::vector<std::pair<int, int>>& path)
{
    for (size_t i = 0; i < path.size(); ++i)
    {
        const auto& p_node = path[i];
        float x_center = render_offset_x + p_node.second * pentagon_dx + (p_node.first % 2) * (pentagon_dx / 2.0f);
        float y_center = render_offset_y + p_node.first * pentagon_dy;

        // Make path nodes slightly smaller or different shape/style
        DrawPentagon({x_center, y_center}, pentagon_radius * path_radius_factor, color_path_solution);

        // Optionally draw numbers on path nodes
        // std::string num_str = std::to_string(i);
        // DrawText(num_str.c_str(), x_center - MeasureText(num_str.c_str(),10)/2, y_center - 5, 10, BLACK);
    }
}

// Note: The pentagon drawing functions (DrawPentagon) are assumed to be available globally
// or via the "pentagon.h" include. If they are methods of a Pentagon class,
// an instance of Pentagon or static calls would be needed.
// The current structure with a global DrawPentagon function is fine.
// The renderer relies on the Board instance passed at construction (or updated)
// to get critical layout information like pentagon_radius, dx, dy, and render_offsets.
// If these can change (e.g. different mazes, resizable window), the Renderer might need
// a way to refresh these values from the Board.
// For now, they are set at Renderer construction.
// The `board_to_draw` parameter in methods like DrawBoard allows flexibility if multiple
// boards were ever a concept, but typically it will be `this->board`.
// Using `current_board` parameter name for clarity in DrawGame.
// The UI_PANEL_WIDTH is hardcoded here but could be a configuration.
// Colors are also hardcoded; a ColorTheme struct or similar could make them configurable.
// The calculation of x_center, y_center for pentagons is based on the original main.cpp's
// staggered row layout. This will be a key area of change when implementing true pentagonal adjacency
// for movement, as the drawing coordinates must match the logical grid model.
// If the logical grid model changes (e.g. axial coordinates for hex/pentagonal grids),
// the coordinate conversion for drawing will also need to change.
// For now, this maintains the existing visual representation.
// Added more descriptive color names and separated some UI text for clarity.
// Added game over messages to be drawn by the renderer.
// Path drawing is now more distinct.
// Ensured UI panel is drawn first so game elements can overlay parts of it if necessary (though usually not).
// Game messages (win/lose) are drawn last, on top of everything.
// Path solution is drawn before entities, so entities appear on top of the path highlight.
// Dynamic wall turn countdown is drawn.
// UI provides more game instructions.
