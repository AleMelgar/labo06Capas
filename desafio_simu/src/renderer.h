#pragma once

#include "config.h"
#include "grid_manager.h"
#include "player_entity.h"
#include <raylib.h>
#include <vector>
#include <string> // Para std::string en DrawText

class Renderer
{
public:
    Renderer(int screen_w, int screen_h, const std::string& window_title);
    ~Renderer();

    void BeginDrawingSequence();
    void EndDrawingSequence();

    void DrawGrid(const GridManager& grid, int current_turn, float start_x, float start_y) const;
    // Se pasa la fila de la entidad para determinar su rotación.
    void DrawEntity(const Entity& entity, int entity_row, float start_x, float start_y, float radius_scale, Color color, const std::string& label) const;
    void DrawPath(const std::vector<Position>& path, float start_x, float start_y, float radius_scale, Color color) const;
    void DrawUI(int current_turn, GameState game_state) const;

private:
    // Modificado para aceptar rotación
    void DrawPentagonCell(Vector2 center, float radius, float rotation, Color color) const;
    // Podríamos tener aquí los colores si no están en config.h
};
