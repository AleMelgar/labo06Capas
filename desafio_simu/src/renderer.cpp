#include "renderer.h"
#include "config.h" // Para colores y constantes de pentágono
#include <raylib.h>
// #include "pentagon.h" // Ya no es necesario si DrawLocalPentagon está aquí

namespace { // Espacio anónimo para helpers locales a este archivo
    // Modificado para aceptar rotación
    void DrawLocalPentagon(Vector2 center, float radius, float rotation, Color color) {
        DrawPoly(center, 5, radius, rotation, color);
    }
}

Renderer::Renderer(int screen_w, int screen_h, const std::string& window_title) {
    InitWindow(screen_w, screen_h, window_title.c_str());
    SetTargetFPS(60);
}

Renderer::~Renderer() {
    CloseWindow();
}

void Renderer::BeginDrawingSequence() {
    BeginDrawing();
    ClearBackground(DARK_GREEN);
}

void Renderer::EndDrawingSequence() {
    EndDrawing();
}

// Implementación del método de clase modificado
void Renderer::DrawPentagonCell(Vector2 center, float radius, float rotation, Color color) const {
    DrawLocalPentagon(center, radius, rotation, color);
}

void Renderer::DrawGrid(const GridManager& grid, int current_turn, float start_x, float start_y) const {
    for (int r = 0; r < grid.GetRows(); ++r) {
        for (int c = 0; c < grid.GetCols(); ++c) {
            float pent_center_x = start_x + c * PENTAGON_DX + (r % 2) * (PENTAGON_DX / 2.0f);
            float pent_center_y = start_y + r * PENTAGON_DY;
            
            Color cell_color = RAYWHITE; // Color por defecto para PATH
            int cell_type_val = grid.grid_data[r][c];

            if (cell_type_val == static_cast<int>(CellType::WALL)) {
                cell_color = WALL_GRAY;
            } else if (cell_type_val == static_cast<int>(CellType::ALTERNATING_WALL)) {
                cell_color = (current_turn % 2 == 0) ? EVEN_CELL_COLOR : ODD_CELL_COLOR;
            } else if (cell_type_val == static_cast<int>(CellType::PATH)) {
                 // Ya es RAYWHITE por defecto
            } else if (cell_type_val == static_cast<int>(CellType::DYNAMIC_WALL)) {
                bool found_in_dynamic_list = false;
                for (const auto& dw : grid.dynamic_walls_list) {
                    if (dw.row == r && dw.col == c && dw.turns_to_open > 0) {
                        cell_color = DYNAMIC_WALL_COLOR;
                        DrawText(TextFormat("%d", dw.turns_to_open), 
                                 pent_center_x - MeasureText(TextFormat("%d", dw.turns_to_open), 15) / 2.0f,
                                 pent_center_y - 7, 15, WHITE);
                        found_in_dynamic_list = true;
                        break;
                    }
                }
                if (!found_in_dynamic_list) {
                    cell_color = RAYWHITE; // Abierta o inconsistente, dibujar como abierta
                }
            }
            
            // Determinar rotación basado en la fila
            float rotation = (r % 2 == 0) ? -90.0f : 90.0f; // Fila par: punta arriba, Fila impar: punta abajo
            DrawPentagonCell({pent_center_x, pent_center_y}, PENTAGON_RADIUS, rotation, cell_color);
        }
    }
}

// Modificado para tomar entity_row para la rotación
void Renderer::DrawEntity(const Entity& entity, int entity_row, float start_x, float start_y, float radius_scale, Color color, const std::string& label) const {
    if (!entity.IsActive()) return;

    float entity_center_x = start_x + entity.GetCol() * PENTAGON_DX + (entity_row % 2) * (PENTAGON_DX / 2.0f);
    float entity_center_y = start_y + entity_row * PENTAGON_DY;

    // Determinar rotación basado en la fila de la entidad
    float rotation = (entity_row % 2 == 0) ? -90.0f : 90.0f;
    DrawPentagonCell({entity_center_x, entity_center_y}, PENTAGON_RADIUS * radius_scale, rotation, color);
    
    if (!label.empty()) {
        DrawText(label.c_str(), 
                 entity_center_x - MeasureText(label.c_str(), 14) / 2.0f,
                 entity_center_y - PENTAGON_RADIUS * radius_scale - 12,
                 14, color);
    }
}

void Renderer::DrawPath(const std::vector<Position>& path, float start_x, float start_y, float radius_scale, Color color) const {
    for (const auto& p : path) { // p es Position {fila, columna}
        int path_row = p.first;
        int path_col = p.second;
        float pent_center_x = start_x + path_col * PENTAGON_DX + (path_row % 2) * (PENTAGON_DX / 2.0f);
        float pent_center_y = start_y + path_row * PENTAGON_DY;
        
        // Determinar rotación basado en la fila del camino
        float rotation = (path_row % 2 == 0) ? -90.0f : 90.0f;
        DrawPentagonCell({pent_center_x, pent_center_y}, PENTAGON_RADIUS * radius_scale, rotation, color);
    }
}

void Renderer::DrawUI(int current_turn, GameState game_state) const {
    DrawRectangle(0, 0, 230, SCREEN_HEIGHT, UI_PANEL_COLOR);
    DrawText("ESCAPE THE GRID", 20, 20, 22, ACCENT_BLUE);
    DrawText(TextFormat("Turno: %d", current_turn), 20, 60, 18, TEXT_WHITE);
    
    DrawText("Controles:", 20, 100, 18, TEXT_WHITE);
    DrawText("- A, D: Izquierda, Derecha", 20, 130, 16, TEXT_WHITE);
    DrawText("- Q, W, E: Mov. Arriba", 20, 150, 16, TEXT_WHITE);
    DrawText("- Z, X, C: Mov. Abajo", 20, 170, 16, TEXT_WHITE);
    DrawText("- S: Mostrar/Ocultar Solución", 20, 190, 16, TEXT_WHITE); // Ajustar Y
    
    DrawText("Objetivo:", 20, 210, 18, TEXT_WHITE); // Ajustado Y
    DrawText("- Alcanza la esquina inferior derecha.", 20, 240, 14, GRAY); // Ajustado Y
    DrawText("- Evita al Clon.", 20, 260, 14, GRAY); // Ajustado Y

    if (game_state == GameState::GAME_OVER) {
        // Determinar si fue victoria o derrota
        // Esto es un placeholder, necesitaría más lógica desde Game para saber si ganó o perdió.
        // Por ahora, asumo que si es GAME_OVER es porque escapó.
        DrawText("¡HAS ESCAPADO!", 20, SCREEN_HEIGHT / 2.0f + 40, 22, PLAYER_GREEN);
    }
     DrawText("R: Reiniciar", 20, SCREEN_HEIGHT - 40, 16, TEXT_WHITE);
}
