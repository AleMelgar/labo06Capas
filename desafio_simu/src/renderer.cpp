#include "renderer.h"
#include "config.h" // Para colores y constantes de pentágono
#include <raylib.h>
#include <cmath>   // Para cosf y sinf
// #include "pentagon.h" // Ya no es necesario si DrawLocalPentagon está aquí

namespace { // Espacio anónimo para helpers locales a este archivo
    // Modificado para aceptar rotación
    // DrawLocalPentagon será reemplazado/eliminado cuando DrawCustomPentagon sea usado por DrawPentagonCell.
    void DrawLocalPentagon(Vector2 center, float radius, float rotation, Color color) {
        DrawPoly(center, 5, radius, rotation, color);
    }
}

// Implementación de DrawCustomPentagon
void Renderer::DrawCustomPentagon(Vector2 center, float width, float rect_h, float tri_h, float rotation_degrees, Color color) const {
    // a. Calcular los 5 vértices del pentágono "punta arriba" en coordenadas locales (centro (0,0))
    //    La punta del triángulo está en la parte negativa de Y. La base del rectángulo está en la parte positiva de Y.
    //    El centro del pentágono (0,0 local) está en el medio de la altura del rectángulo.
    Vector2 v_local[5];
    v_local[0] = {0.0f, -(rect_h / 2.0f + tri_h)};       // Punta superior del triángulo
    v_local[1] = {width / 2.0f, -rect_h / 2.0f};         // Esquina superior derecha del rectángulo (base del triángulo)
    v_local[2] = {width / 2.0f, rect_h / 2.0f};          // Esquina inferior derecha del rectángulo
    v_local[3] = {-width / 2.0f, rect_h / 2.0f};         // Esquina inferior izquierda del rectángulo
    v_local[4] = {-width / 2.0f, -rect_h / 2.0f};        // Esquina superior izquierda del rectángulo (base del triángulo)

    // b. Convertir rotation_degrees a radianes
    float angle_rad = rotation_degrees * DEG2RAD; // DEG2RAD es una macro de Raylib
    float cos_a = cosf(angle_rad);
    float sin_a = sinf(angle_rad);

    Vector2 v_final[5]; // Vértices finales rotados y trasladados

    // c. Rotar cada vértice alrededor de (0,0) y d. Trasladar al 'center' global
    for (int i = 0; i < 5; ++i) {
        float rotated_x = v_local[i].x * cos_a - v_local[i].y * sin_a;
        float rotated_y = v_local[i].x * sin_a + v_local[i].y * cos_a;
        v_final[i] = {center.x + rotated_x, center.y + rotated_y};
    }

    // e. Dibujar el pentágono relleno
    // Triángulo superior (techo)
    DrawTriangle(v_final[0], v_final[1], v_final[4], color);
    // Parte rectangular (se puede dibujar como dos triángulos)
    // Triángulo 1 del rectángulo: (v1, v2, v3)
    DrawTriangle(v_final[1], v_final[2], v_final[3], color);
    // Triángulo 2 del rectángulo: (v1, v3, v4) - esto forma el quad v1,v2,v3,v4
    DrawTriangle(v_final[1], v_final[3], v_final[4], color);
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
void Renderer::DrawPentagonCell(Vector2 center, float scale_factor, int row, Color color) const {
    // Usar las dimensiones base definidas en el Paso 1 del plan, escaladas por scale_factor.
    // PENTAGON_DX y PENTAGON_DY son las dimensiones base para scale_factor = 1.0f.

    // float base_width = PENTAGON_RADIUS * 1.5f; // PENTAGON_DX
    // float base_total_height = PENTAGON_RADIUS * 1.3f; // PENTAGON_DY

    // Las constantes PENTAGON_DX y PENTAGON_DY ya están disponibles desde config.h
    float cell_w = PENTAGON_DX * scale_factor;
    float total_h = PENTAGON_DY * scale_factor; // Altura total visual del pentágono

    // Proporciones para las partes rectangular y triangular (como se definió en el Paso 1)
    float rect_h_part = total_h * 0.6f;
    float tri_h_part = total_h * 0.4f;

    // Determinar rotación para DrawCustomPentagon: 0 para punta arriba, 180 para punta abajo
    float rotation_for_custom = (row % 2 == 0) ? 0.0f : 180.0f;

    DrawCustomPentagon(center, cell_w, rect_h_part, tri_h_part, rotation_for_custom, color);

    // DrawLocalPentagon ya no se usa aquí. Puede ser eliminada si no se usa en ningún otro lado.
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

            // La rotación ahora la maneja DrawPentagonCell internamente usando la fila 'r'.
            // El radio base PENTAGON_RADIUS se usa implicitamente en DrawPentagonCell a través de PENTAGON_DX/DY.
            // Para el grid, el factor de escala es 1.0f.
            DrawPentagonCell({pent_center_x, pent_center_y}, 1.0f, r, cell_color);
        }
    }
}

// Modificado para tomar entity_row para la rotación
void Renderer::DrawEntity(const Entity& entity, int entity_row, float start_x, float start_y, float radius_scale, Color color, const std::string& label) const {
    if (!entity.IsActive()) return;

    float entity_center_x = start_x + entity.GetCol() * PENTAGON_DX + (entity_row % 2) * (PENTAGON_DX / 2.0f);
    float entity_center_y = start_y + entity_row * PENTAGON_DY;

    // La rotación la maneja DrawPentagonCell.
    // PENTAGON_RADIUS * radius_scale se convierte en solo radius_scale como el factor de escala.
    DrawPentagonCell({entity_center_x, entity_center_y}, radius_scale, entity_row, color);

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

        // La rotación la maneja DrawPentagonCell usando path_row.
        // PENTAGON_RADIUS * radius_scale se convierte en solo radius_scale como el factor de escala.
        DrawPentagonCell({pent_center_x, pent_center_y}, radius_scale, path_row, color);
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
