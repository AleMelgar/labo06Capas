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
    void DrawEntity(const Entity& entity, float start_x, float start_y, float radius_scale, Color color, const std::string& label) const;
    void DrawPath(const std::vector<Position>& path, float start_x, float start_y, float radius_scale, Color color) const;
    void DrawUI(int current_turn, GameState game_state) const;

private:
    void DrawPentagonCell(Vector2 center, float radius, Color color) const; // Wrapper para DrawPentagon
    // Podríamos tener aquí los colores si no están en config.h
};
