#include "renderer.h"
#include "config.h" // Para colores y constantes, y PENTAGON_SIDE_LENGTH_TEMP etc.
#include <raylib.h>
#include <vector>
#include <cmath>
#include <string> // Para std::string en DrawText

// Se define M_PI si no está definido (aunque cmath debería proporcionarlo)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Las constantes PENTAGON_SIDE_LENGTH, PENTAGON_DX, PENTAGON_DY ahora vienen de config.h
// por lo que las definiciones temporales _TEMP aquí ya no son necesarias.

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

void Renderer::DrawPentagonCell(Vector2 center, float side_length, float rotation_degrees, Color fillColor, Color lineColor) const {
    float base_width = side_length;
    float wall_height = side_length;
    float roof_peak_offset_y = (std::sqrt(3.0f) / 2.0f) * side_length;

    Vector2 points[5];

    Vector2 v_base_left_up = {-base_width / 2.0f, 0.0f};
    Vector2 v_base_right_up = {base_width / 2.0f, 0.0f};
    Vector2 v_top_right_up = {base_width / 2.0f, -wall_height};
    Vector2 v_peak_up = {0.0f, -wall_height - roof_peak_offset_y};
    Vector2 v_top_left_up = {-base_width / 2.0f, -wall_height};

    Vector2 v_base_left_down = {-base_width / 2.0f, 0.0f};
    Vector2 v_base_right_down = {base_width / 2.0f, 0.0f};
    Vector2 v_wall_right_down = {base_width / 2.0f, wall_height};
    Vector2 v_peak_down = {0.0f, wall_height + roof_peak_offset_y};
    Vector2 v_wall_left_down = {-base_width / 2.0f, wall_height};

    if (rotation_degrees == 0.0f) {
        points[0] = v_base_left_up;
        points[1] = v_base_right_up;
        points[2] = v_top_right_up;
        points[3] = v_peak_up;
        points[4] = v_top_left_up;
    } else {
        points[0] = v_base_left_down;
        points[1] = v_base_right_down;
        points[2] = v_wall_right_down;
        points[3] = v_peak_down;
        points[4] = v_wall_left_down;
    }

    for (int i = 0; i < 5; ++i) {
        points[i] = Vector2Add(points[i], center);
    }

    DrawTriangle(points[0], points[1], points[2], fillColor);
    DrawTriangle(points[0], points[2], points[3], fillColor);
    DrawTriangle(points[0], points[3], points[4], fillColor);

    DrawLineV(points[0], points[1], lineColor);
    DrawLineV(points[1], points[2], lineColor);
    DrawLineV(points[2], points[3], lineColor);
    DrawLineV(points[3], points[4], lineColor);
    DrawLineV(points[4], points[0], lineColor);
}

void Renderer::DrawGrid(const GridManager& grid, int current_turn, float start_x, float start_y) const {
    for (int r = 0; r < grid.GetRows(); ++r) {
        for (int c = 0; c < grid.GetCols(); ++c) {
            // Usar las constantes definitivas de config.h
            float pent_center_base_x = start_x + c * PENTAGON_DX + (r % 2 != 0) * (PENTAGON_DX / 2.0f); // Desplazamiento para filas impares (r=1, 3, 5...)
            float pent_center_base_y = start_y + r * PENTAGON_DY;

            Color cell_fill_color = RAYWHITE;
            Color cell_line_color = BLACK;   // Bordes negros para visibilidad
            int cell_type_val = grid.grid_data[r][c];

            if (cell_type_val == static_cast<int>(CellType::WALL)) {
                cell_fill_color = WALL_GRAY;
            } else if (cell_type_val == static_cast<int>(CellType::ALTERNATING_WALL)) {
                cell_fill_color = (current_turn % 2 == 0) ? EVEN_CELL_COLOR : ODD_CELL_COLOR;
            } else if (cell_type_val == static_cast<int>(CellType::PATH)) {
                // RAYWHITE por defecto
            } else if (cell_type_val == static_cast<int>(CellType::DYNAMIC_WALL)) {
                bool found_in_dynamic_list = false;
                for (const auto& dw : grid.dynamic_walls_list) {
                    if (dw.row == r && dw.col == c && dw.turns_to_open > 0) {
                        cell_fill_color = DYNAMIC_WALL_COLOR;

                        float text_x = pent_center_base_x - MeasureText(TextFormat("%d", dw.turns_to_open), 15) / 2.0f;
                        float text_y_offset = 0; // El offset vertical del texto dependerá de la orientación
                        if (r < 2) { // Punta arriba
                            text_y_offset = -PENTAGON_SIDE_LENGTH * 0.5f; // Dentro de la parte rectangular, hacia arriba
                        } else { // Punta abajo
                            text_y_offset = PENTAGON_SIDE_LENGTH * 0.5f; // Dentro de la parte rectangular, hacia abajo
                        }
                        DrawText(TextFormat("%d", dw.turns_to_open), text_x, pent_center_base_y + text_y_offset - 7, 15, WHITE); // -7 para centrar mejor la fuente

                        found_in_dynamic_list = true;
                        break;
                    }
                }
                if (!found_in_dynamic_list) {
                    cell_fill_color = RAYWHITE;
                }
            }

            // Lógica de rotación según el plan:
            // Filas 0 y 1 (las dos primeras) hacia arriba (0 grados).
            // Filas 2 en adelante, hacia abajo (180 grados).
            float rotation = (r < 2) ? 0.0f : 180.0f;

            DrawPentagonCell({pent_center_base_x, pent_center_base_y}, PENTAGON_SIDE_LENGTH, rotation, cell_fill_color, cell_line_color);
        }
    }
}

void Renderer::DrawEntity(const Entity& entity, int entity_row, float start_x, float start_y, float radius_scale, Color color, const std::string& label) const {
    if (!entity.IsActive()) return;

    // Usar las constantes definitivas de config.h
    float entity_center_base_x = start_x + entity.GetCol() * PENTAGON_DX + (entity_row % 2 != 0) * (PENTAGON_DX / 2.0f);
    float entity_center_base_y = start_y + entity_row * PENTAGON_DY;

    // Lógica de rotación: primeras dos filas (0, 1) hacia arriba, resto hacia abajo.
    float rotation = (entity_row < 2) ? 0.0f : 180.0f;
    float scaled_side_length = PENTAGON_SIDE_LENGTH * radius_scale;
    DrawPentagonCell({entity_center_base_x, entity_center_base_y}, scaled_side_length, rotation, color, BLACK); // Borde negro para entidad

    if (!label.empty()) {
        float text_y_position;
        // Calcular la altura total de la casa para posicionar el texto correctamente
        // Altura = wall_height + roof_peak_offset_y = scaled_side_length + (sqrt(3)/2)*scaled_side_length
        float house_total_height = scaled_side_length + (std::sqrt(3.0f) / 2.0f) * scaled_side_length;

        if (rotation == 0.0f) { // Punta arriba, el centro Y de la base está en entity_center_base_y
            // El texto debe ir por encima del pico. El pico está en entity_center_base_y - house_total_height (aprox, si el centro fuera la punta)
            // Si entity_center_base_y es la base, el pico está en entity_center_base_y - (wall_height + roof_peak_offset_y)
            text_y_position = entity_center_base_y - (scaled_side_length + (std::sqrt(3.0f) / 2.0f) * scaled_side_length) - 12; // 12 es un offset adicional
        } else { // Punta abajo, el centro Y de la base (que está arriba) está en entity_center_base_y
            // El texto debe ir por "encima" de la figura, que ahora es encima de la base (que está arriba).
            text_y_position = entity_center_base_y - 12; // 12 es un offset para que esté por encima de la línea base
        }
        DrawText(label.c_str(),
                 entity_center_base_x - MeasureText(label.c_str(), 14) / 2.0f, // Centrado horizontalmente
                 text_y_position,
                 14, WHITE); // Color de texto (puede ser configurable o igual a 'color')
    }
}

void Renderer::DrawPath(const std::vector<Position>& path, float start_x, float start_y, float radius_scale, Color color) const {
    for (const auto& p : path) {
        int path_row = p.first;
        int path_col = p.second;
        // Usar las constantes definitivas de config.h
        float pent_center_base_x = start_x + path_col * PENTAGON_DX + (path_row % 2 != 0) * (PENTAGON_DX / 2.0f);
        float pent_center_base_y = start_y + path_row * PENTAGON_DY;

        // Lógica de rotación: primeras dos filas (0, 1) hacia arriba, resto hacia abajo.
        float rotation = (path_row < 2) ? 0.0f : 180.0f;
        DrawPentagonCell({pent_center_base_x, pent_center_base_y}, PENTAGON_SIDE_LENGTH * radius_scale, rotation, color, DARKGRAY); // Borde gris oscuro para el camino
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
    DrawText("- S: Mostrar/Ocultar Solución", 20, 190, 16, TEXT_WHITE);

    DrawText("Objetivo:", 20, 210, 18, TEXT_WHITE);
    DrawText("- Alcanza la esquina inferior derecha.", 20, 240, 14, GRAY);
    DrawText("- Evita al Clon.", 20, 260, 14, GRAY);

    if (game_state == GameState::GAME_OVER) {
        // Esta parte necesitaría lógica de Game para determinar victoria/derrota
        DrawText("¡HAS ESCAPADO!", 20, SCREEN_HEIGHT / 2.0f + 40, 22, PLAYER_GREEN);
    }
     DrawText("R: Reiniciar", 20, SCREEN_HEIGHT - 40, 16, TEXT_WHITE);
}
