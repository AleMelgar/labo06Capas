#include "grid_manager.h"
#include "config.h" // Para CellType, DynamicWall, etc.
#include <fstream>
#include <sstream>
#include <iostream> // Para std::cerr y std::cout (debug)
#include <queue>
#include <algorithm> // Para std::reverse
#include <set>       // Para el BFS con estados (si se usa)
#include <tuple>     // Para std::tuple en BFS

GridManager::GridManager() : num_rows(0), num_cols(0)
{
    // El constructor puede quedar vacío si LoadMaze hace toda la inicialización.
}

bool GridManager::LoadMaze(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: No se pudo abrir el archivo de laberinto: " << filename << std::endl;
        return false;
    }

    grid_data.clear();
    dynamic_walls_list.clear();
    std::string line_str;
    int r_idx = 0;
    while (std::getline(file, line_str))
    {
        std::vector<int> row_vec;
        std::stringstream ss(line_str);
        int cell_val;
        int c_idx = 0;
        while (ss >> cell_val)
        {
            row_vec.push_back(cell_val);
            if (cell_val == static_cast<int>(CellType::DYNAMIC_WALL))
            {
                // Asumimos que todas las paredes dinámicas inician con 3 turnos para abrirse,
                // como en el main.cpp original. Esto podría ser configurable por archivo.
                dynamic_walls_list.push_back({r_idx, c_idx, 3, CellType::DYNAMIC_WALL});
            }
            c_idx++;
        }
        if (!row_vec.empty())
        {
            grid_data.push_back(row_vec);
        }
        r_idx++;
    }
    file.close();

    if (grid_data.empty() || grid_data[0].empty())
    {
        std::cerr << "Error: El laberinto está vacío o mal formateado." << std::endl;
        num_rows = 0;
        num_cols = 0;
        return false;
    }

    num_rows = grid_data.size();
    num_cols = grid_data[0].size();
    return true;
}

int GridManager::GetCellType(int r, int c) const
{
    if (r < 0 || r >= num_rows || c < 0 || c >= num_cols)
    {
        return -1; // Fuera de límites, podría ser un tipo especial de error.
    }
    return grid_data[r][c];
}

bool GridManager::IsCellWalkable(int r, int c, int current_turn, bool is_player, Position clone_pos, bool clone_is_active) const
{
    if (r < 0 || r >= num_rows || c < 0 || c >= num_cols)
    {
        return false; // Fuera de los límites del grid
    }

    // Comprobar colisión con el clon si es el jugador quien se mueve
    if (is_player && clone_is_active && r == clone_pos.first && c == clone_pos.second)
    {
        return false;
    }

    int cell_type_val = grid_data[r][c];

    // Celda es pared fija
    if (cell_type_val == static_cast<int>(CellType::WALL))
    {
        return false;
    }

    // Celda es pared alternante
    if (cell_type_val == static_cast<int>(CellType::ALTERNATING_WALL))
    {
        // Es pared si el turno es impar (bloquea), transitable si es par (abierta)
        // La lógica original era: (cellType == 2 && current_turn % 2 != 0) -> cellIsBlocked = true;
        // Por lo tanto, es caminable si current_turn % 2 == 0
        return (current_turn % 2 == 0);
    }

    // Celda es pared dinámica
    if (cell_type_val == static_cast<int>(CellType::DYNAMIC_WALL))
    {
        for (const auto &dw : dynamic_walls_list)
        {
            if (dw.row == r && dw.col == c)
            {
                return dw.turns_to_open <= 0; // Caminable si ya se abrió
            }
        }
        // Si no está en la lista (no debería pasar si se cargó bien), o por defecto
        return false; // Considerarla cerrada si hay un problema
    }

    // Celda es camino (PATH) u otro tipo no definido como bloqueante
    return true;
}

void GridManager::UpdateDynamicWalls(int current_turn)
{
    bool needs_update = false;
    for (auto &dw : dynamic_walls_list)
    {
        if (dw.turns_to_open > 0)
        {
            dw.turns_to_open--;
            if (dw.turns_to_open == 0)
            {
                // Cambiar el tipo de celda en grid_data para que sea permanentemente abierta
                if (dw.row >= 0 && dw.row < num_rows && dw.col >= 0 && dw.col < num_cols)
                {
                    grid_data[dw.row][dw.col] = static_cast<int>(CellType::PATH);
                    needs_update = true;
                }
            }
        }
    }
    // Si se necesitara alguna lógica adicional cuando una pared se abre, iría aquí.
    // if (needs_update) { /* por ejemplo, recalcular algo */ }
}

// ...existing code...

// Implementación de IsCellOpenForPathfinding (usado por BFS)
bool GridManager::IsCellOpenForPathfinding(
    int r, int c,
    const MazeGrid &temp_grid_state_for_bfs,
    const std::vector<std::vector<bool>> &visited,
    const std::vector<DynamicWall> &dynamic_walls_at_bfs_start,
    int turn_in_path,
    int current_turn_for_calc // <-- Añadido aquí
) const
{
    if (r < 0 || r >= num_rows || c < 0 || c >= num_cols || visited[r][c])
    {
        return false;
    }

    int cell_type_val = temp_grid_state_for_bfs[r][c];

    if (cell_type_val == static_cast<int>(CellType::WALL))
    {
        return false;
    }
    if (cell_type_val == static_cast<int>(CellType::ALTERNATING_WALL))
    {
        return (turn_in_path % 2 == 0);
    }
    if (cell_type_val == static_cast<int>(CellType::DYNAMIC_WALL))
    {
        for (const auto &dw_snapshot : dynamic_walls_at_bfs_start)
        {
            if (dw_snapshot.row == r && dw_snapshot.col == c)
            {
                int turns_faltantes_al_inicio_bfs = dw_snapshot.turns_to_open;
                int pasos_dados_en_bfs = turn_in_path - current_turn_for_calc;
                if (turns_faltantes_al_inicio_bfs > pasos_dados_en_bfs)
                {
                    return false;
                }
                break; // Ya encontramos la pared, no seguimos buscando
            }
        }
        // Si no está en la lista, o ya se abrió, es transitable.
    }
    return true;
}
// ...existing code...

std::vector<Position> GridManager::CalculateShortestPath(Position start_pos, Position end_pos, int current_turn_for_calc) const
{
    std::vector<Position> path;
    if (num_rows == 0 || num_cols == 0)
        return path;
    if (start_pos == end_pos)
    {
        path.push_back(start_pos);
        return path;
    }

    std::vector<std::vector<bool>> visited(num_rows, std::vector<bool>(num_cols, false));
    std::vector<std::vector<Position>> predecessor(num_rows, std::vector<Position>(num_cols, {-1, -1}));

    // Cola para <fila, columna, turno_absoluto_del_juego>
    std::queue<std::tuple<int, int, int>> q;

 // ...existing code...
if (!IsCellOpenForPathfinding(start_pos.first, start_pos.second, grid_data, visited, dynamic_walls_list, current_turn_for_calc, current_turn_for_calc))
{
    return path; // Posición inicial no es válida en el turno actual.
}
// ...existing code...

    q.push({start_pos.first, start_pos.second, current_turn_for_calc});
    visited[start_pos.first][start_pos.second] = true;

    // Movimientos posibles (arriba, abajo, izquierda, derecha)
    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};

    bool found_path = false;
    while (!q.empty())
    {
        auto [r, c, turn_at_current_cell] = q.front();
        q.pop();

        if (r == end_pos.first && c == end_pos.second)
        {
            found_path = true;
            break;
        }

        int next_turn_for_neighbor = turn_at_current_cell + 1;

        for (int i = 0; i < 4; ++i)
        {
            int nr = r + dr[i];
            int nc = c + dc[i];

            if (IsCellOpenForPathfinding(nr, nc, grid_data, visited, dynamic_walls_list, next_turn_for_neighbor, current_turn_for_calc))
            {
                visited[nr][nc] = true;
                predecessor[nr][nc] = {r, c};
                q.push({nr, nc, next_turn_for_neighbor});
            }
        }
    }

    if (found_path)
    {
        Position current = end_pos;
        while (current.first != -1 && current.second != -1 && current != start_pos)
        {
            path.push_back(current);
            current = predecessor[current.first][current.second];
        }
        path.push_back(start_pos);
        std::reverse(path.begin(), path.end());
    }

    return path;
}
