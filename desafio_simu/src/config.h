#pragma once

#include <raylib.h>
#include <vector>
#include <string>
#include <cmath> // Para std::sqrt

// Constantes del juego
constexpr int SCREEN_WIDTH = 900;
constexpr int SCREEN_HEIGHT = 600;
constexpr int CLONE_ACTIVATION_TURNS = 6;

// --- NUEVAS CONSTANTES PARA EL PENTÁGONO-CASA ---
// Longitud base de los lados rectos y de la base horizontal de la casa.
constexpr float PENTAGON_SIDE_LENGTH = 30.0f;

// Dimensiones calculadas de la casa-pentágono
constexpr float PENTAGON_HOUSE_BASE_WIDTH = PENTAGON_SIDE_LENGTH;
constexpr float PENTAGON_HOUSE_WALL_HEIGHT = PENTAGON_SIDE_LENGTH;
// Altura del techo, asumiendo que los lados del techo forman un triángulo equilátero
// con la línea imaginaria que une la parte superior de las paredes.
// Altura de un triángulo equilátero de lado 's' es (sqrt(3)/2)*s.
constexpr float PENTAGON_HOUSE_ROOF_HEIGHT = (PENTAGON_SIDE_LENGTH * 0.86602540378f); // sqrt(3)/2 * PENTAGON_SIDE_LENGTH

// Ancho total de la celda (es el ancho de la base)
constexpr float PENTAGON_HOUSE_WIDTH = PENTAGON_HOUSE_BASE_WIDTH;
// Altura total de la celda (desde la base hasta el pico del techo)
constexpr float PENTAGON_HOUSE_TOTAL_HEIGHT = PENTAGON_HOUSE_WALL_HEIGHT + PENTAGON_HOUSE_ROOF_HEIGHT;

// --- ESPACIADO DE LA MALLA (GRID DX, DY) ---
// PENTAGON_DX: Distancia horizontal entre los puntos de anclaje (centro de la base) de celdas en la misma fila.
constexpr float PENTAGON_DX = PENTAGON_HOUSE_WIDTH;

// PENTAGON_DY: Distancia vertical entre los puntos de anclaje (centro de la base) de filas adyacentes.
// Para un encaje tipo panal, donde las bases de una fila se alinean aproximadamente con los techos de la fila anterior/siguiente:
// La base de una celda en la fila R está en Y_R.
// La base de una celda en la fila R+1 está en Y_R + PENTAGON_DY.
// Si la fila R es punta arriba, su "techo" (paredes + techo) ocupa desde Y_R hasta Y_R - PENTAGON_HOUSE_TOTAL_HEIGHT.
// Si la fila R+1 es punta abajo, su "techo" (paredes + techo, invertido) ocupa desde Y_R+PENTAGON_DY hasta Y_R+PENTAGON_DY + PENTAGON_HOUSE_TOTAL_HEIGHT.
// Para que encajen bien, la parte inferior del cuerpo de la casa (base - altura de pared) de una fila
// debe estar cerca del pico de la fila adyacente.
// Si DY = altura de la pared + mitad de la altura del techo, las celdas se intercalarán.
constexpr float PENTAGON_DY = PENTAGON_HOUSE_WALL_HEIGHT + (PENTAGON_HOUSE_ROOF_HEIGHT / 2.0f);
// Este valor de DY asegura que la parte más ancha de las casas (la base) de una fila
// esté verticalmente separada de la parte más ancha de las casas de la fila adyacente
// por la altura de las paredes más la mitad de la altura del techo, permitiendo que los techos se intercalen.

// Antiguas constantes relacionadas con PENTAGON_RADIUS (comentadas para referencia, pueden eliminarse)
// constexpr float PENTAGON_RADIUS = 45.0f;
// constexpr float OLD_PENTAGON_DX = PENTAGON_RADIUS * 1.5f;
// constexpr float OLD_PENTAGON_DY = PENTAGON_RADIUS * 1.3f;


// Colores
const Color DARK_GREEN = {20, 25, 30, 255}; // Ajustado para mejor contraste con bordes claros
const Color UI_PANEL_COLOR = {45, 50, 60, 255};
const Color TEXT_WHITE = {240, 240, 240, 255};
const Color ACCENT_BLUE = {102, 204, 255, 255};
const Color PLAYER_GREEN = {0, 200, 120, 255};
const Color CLONE_BLUE = {90, 120, 250, 255};
const Color WALL_GRAY = {100, 100, 100, 255};
const Color DYNAMIC_WALL_COLOR = {60, 60, 60, 255};
const Color EVEN_CELL_COLOR = {70, 140, 255, 255};
const Color ODD_CELL_COLOR = {255, 210, 50, 255};
const Color PATH_SOLUTION_COLOR = ACCENT_BLUE;


// Estados del juego
enum class GameState
{
    PLAYING,
    GAME_OVER,
    PATH_SHOWN
};

// Tipos de celdas del laberinto
enum class CellType
{
    PATH = 0,
    WALL = 1,
    ALTERNATING_WALL = 2,
    DYNAMIC_WALL = 3
};

// Estructura para paredes dinámicas
struct DynamicWall
{
    int row, col;
    int turns_to_open;
    CellType original_type = CellType::DYNAMIC_WALL;
};

// Definición para la grid del laberinto
using MazeGrid = std::vector<std::vector<int>>;
using Position = std::pair<int, int>;
using MovementHistory = std::vector<Position>;
