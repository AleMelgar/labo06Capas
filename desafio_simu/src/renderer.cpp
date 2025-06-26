#include "renderer.h"
#include "config.h" // Para colores y constantes de pentágono
#include <raylib.h>
#include "pentagon.h" // Para la función DrawPentagon original si la reutilizamos

// Definición de la función DrawPentagon que estaba en pentagon.cpp
// Podría ser un método privado de Renderer o una función helper en este .cpp
namespace { // Espacio anónimo para helpers locales a este archivo
    void DrawLocalPentagon(Vector2 center, float radius, Color color) {
        DrawPoly(center, 5, radius, -90, color); // -90 para que la punta esté arriba
    }
}

Renderer::Renderer(int screen_w, int screen_h, const std::string& window_title) {
    InitWindow(screen_w, screen_h, window_title.c_str());
    SetTargetFPS(60);
    // maze_render_offset_x y maze_render_offset_y se calcularán en Game y se pasarán a las funciones de dibujo.
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

void Renderer::DrawPentagonCell(Vector2 center, float radius, Color color) const {
    DrawLocalPentagon(center, radius, color);
}

void Renderer::DrawGrid(const GridManager& grid, int current_turn, float start_x, float start_y) const {
    for (int r = 0; r < grid.GetRows(); ++r) {
        for (int c = 0; c < grid.GetCols(); ++c) {
            float pent_center_x = start_x + c * PENTAGON_DX + (r % 2) * (PENTAGON_DX / 2.0f);
            float pent_center_y = start_y + r * PENTAGON_DY;
            
            Color cell_color = RAYWHITE; // Color por defecto para PATH
            int cell_type_val = grid.grid_data[r][c]; // Acceso directo para el tipo base

            if (cell_type_val == static_cast<int>(CellType::WALL)) {
                cell_color = WALL_GRAY;
            } else if (cell_type_val == static_cast<int>(CellType::ALTERNATING_WALL)) {
                cell_color = (current_turn % 2 == 0) ? EVEN_CELL_COLOR : ODD_CELL_COLOR;
            } else if (cell_type_val == static_cast<int>(CellType::PATH)) {
                 // Ya es RAYWHITE por defecto
            } else if (cell_type_val == static_cast<int>(CellType::DYNAMIC_WALL)) {
                // Este tipo de celda en grid_data significa que *originalmente* era una DYNAMIC_WALL
                // pero aún no se ha abierto permanentemente (convertido a PATH).
                // Necesitamos consultar la lista de dynamic_walls para el contador.
                bool found_in_dynamic_list = false;
                for (const auto& dw : grid.dynamic_walls_list) {
                    if (dw.row == r && dw.col == c && dw.turns_to_open > 0) {
                        cell_color = DYNAMIC_WALL_COLOR;
                        DrawText(TextFormat("%d", dw.turns_to_open), 
                                 pent_center_x - MeasureText(TextFormat("%d", dw.turns_to_open), 15) / 2.0f, // Centrar texto
                                 pent_center_y - 7, // Ajustar posición y
                                 15, WHITE);
                        found_in_dynamic_list = true;
                        break;
                    }
                }
                if (!found_in_dynamic_list) {
                    // Si no está en la lista o turns_to_open es 0, debería ser PATH.
                    // Esto puede ocurrir si UpdateDynamicWalls ya cambió grid_data[r][c] a PATH.
                    // Si grid_data[r][c] sigue siendo DYNAMIC_WALL pero no está en la lista activa,
                    // es un estado inconsistente o ya se abrió. Por seguridad, la dibujamos como abierta.
                    cell_color = RAYWHITE;
                }
            }
            DrawPentagonCell({pent_center_x, pent_center_y}, PENTAGON_RADIUS, cell_color);
        }
    }
}

void Renderer::DrawEntity(const Entity& entity, float start_x, float start_y, float radius_scale, Color color, const std::string& label) const {
    if (!entity.IsActive()) return;

    float entity_center_x = start_x + entity.GetCol() * PENTAGON_DX + (entity.GetRow() % 2) * (PENTAGON_DX / 2.0f);
    float entity_center_y = start_y + entity.GetRow() * PENTAGON_DY;

    DrawPentagonCell({entity_center_x, entity_center_y}, PENTAGON_RADIUS * radius_scale, color);
    
    if (!label.empty()) {
        DrawText(label.c_str(), 
                 entity_center_x - MeasureText(label.c_str(), 14) / 2.0f, // Centrar texto
                 entity_center_y - PENTAGON_RADIUS * radius_scale - 12, // Encima del pentágono
                 14, color);
    }
}

void Renderer::DrawPath(const std::vector<Position>& path, float start_x, float start_y, float radius_scale, Color color) const {
    for (const auto& p : path) {
        float pent_center_x = start_x + p.second * PENTAGON_DX + (p.first % 2) * (PENTAGON_DX / 2.0f);
        float pent_center_y = start_y + p.first * PENTAGON_DY;
        DrawPentagonCell({pent_center_x, pent_center_y}, PENTAGON_RADIUS * radius_scale, color);
    }
}

void Renderer::DrawUI(int current_turn, GameState game_state) const {
    // Panel lateral (similar al original)
    DrawRectangle(0, 0, 230, SCREEN_HEIGHT, UI_PANEL_COLOR);
    DrawText("ESCAPE THE GRID", 20, 20, 22, ACCENT_BLUE);
    DrawText(TextFormat("Turno: %d", current_turn), 20, 60, 18, TEXT_WHITE);
    
    DrawText("Controles:", 20, 100, 18, TEXT_WHITE);
    DrawText("- Flechas: Mover", 20, 130, 16, TEXT_WHITE);
    DrawText("- S: Mostrar/Ocultar Solución", 20, 150, 16, TEXT_WHITE);
    
    DrawText("Objetivo:", 20, 190, 18, TEXT_WHITE);
    DrawText("- Alcanza la esquina inferior derecha.", 20, 220, 14, GRAY);
    DrawText("- Evita al Clon.", 20, 240, 14, GRAY);


    if (game_state == GameState::GAME_OVER) {
        DrawText("¡HAS ESCAPADO!", 20, SCREEN_HEIGHT / 2.0f, 22, PLAYER_GREEN);
    }
    // Podríamos añadir más información si es necesario (ej. estado del clon, etc.)
}
