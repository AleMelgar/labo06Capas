#include "pathfinding.h"
#include <algorithm> // For std::reverse
#include <set>       // For visited set in BFS for better lookup performance

namespace Pathfinding
{
    // Helper function to reconstruct path from parent map
    std::vector<std::pair<int, int>> ReconstructPath(
        const std::map<std::pair<int, int>, std::pair<int, int>>& parent_map,
        std::pair<int, int> start_node,
        std::pair<int, int> goal_node)
    {
        std::vector<std::pair<int, int>> path;
        if (parent_map.find(goal_node) == parent_map.end() && start_node != goal_node) {
            return {};
        }
        std::pair<int, int> current = goal_node;
        while (current != start_node)
        {
            path.push_back(current);
            if (parent_map.find(current) == parent_map.end())
            {
                return {};
            }
            current = parent_map.at(current);
             if (path.size() > parent_map.size() + 2 ) return {};
        }
        path.push_back(start_node);
        std::reverse(path.begin(), path.end());
        return path;
    }

    // GetNeighbors function
    std::vector<std::pair<int, int>> GetNeighbors(
        const Board& board,
        std::pair<int, int> current_node,
        int turn_for_eval,
        const std::vector<DynamicWall>& dynamic_wall_states,
        int bfs_start_turn,
        bool is_pentagonal)
    {
        std::vector<std::pair<int, int>> neighbors;
        int r = current_node.first;
        int c = current_node.second;

        std::vector<std::pair<int, int>> potential_deltas;

        if (is_pentagonal)
        {
            bool is_even_row = (r % 2 == 0);
            if (is_even_row) { // Deltas for EVEN rows (player.row % 2 == 0)
                potential_deltas = {
                    {0, -1},  // A (Left)
                    {0, +1},  // D (Right)
                    {-1, -1}, // Q (Up-Left)
                    {-1, 0},  // W (Up-Center/Right-ish)
                    {+1, 0}   // E (Down)
                };
            } else { // Deltas for ODD rows (player.row % 2 != 0)
                potential_deltas = {
                    {0, -1},  // A (Left)
                    {0, +1},  // D (Right)
                    {-1, 0},  // Q (Up)
                    {+1, 0},  // W (Down-Left-ish)
                    {+1, +1}  // E (Down-Right-ish)
                };
            }
        }
        else // Standard 4-directional grid
        {
            potential_deltas = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; // N, S, W, E
        }

        for (const auto& delta : potential_deltas)
        {
            int nr = r + delta.first;
            int nc = c + delta.second;

            if (nr < 0 || nr >= board.GetRows() || nc < 0 || nc >= board.GetCols()) {
                continue;
            }

            bool is_walkable = true;
            int cell_type = board.GetCellType(nr, nc);

            switch (cell_type) {
                case 0: is_walkable = true; break;
                case 1: is_walkable = false; break;
                case 2: is_walkable = (turn_for_eval % 2 == 0); break;
                case 3:
                {
                    is_walkable = false;
                    bool found_in_list = false;
                    for (const auto& dw : dynamic_wall_states) {
                        if (dw.row == nr && dw.col == nc) {
                            found_in_list = true;
                            int time_elapsed_in_path = turn_for_eval - bfs_start_turn;
                            if (time_elapsed_in_path < 0) time_elapsed_in_path = 0;

                            if (dw.current_turns > 0) {
                                is_walkable = (dw.current_turns - time_elapsed_in_path <= 0);
                            } else {
                                is_walkable = true;
                            }
                            break;
                        }
                    }
                    if (!found_in_list && board.GetCellType(nr,nc) == 3) {
                         is_walkable = false;
                    }
                    break;
                }
                default: is_walkable = false; break;
            }

            if (is_walkable) {
                neighbors.push_back({nr, nc});
            }
        }
        return neighbors;
    }


    std::vector<std::pair<int, int>> FindShortestPathBFS(
        const Board& board,
        std::pair<int, int> start_node,
        std::pair<int, int> goal_node,
        const std::vector<DynamicWall>& dynamic_wall_states,
        int current_game_turn)
    {
        std::queue<std::pair<std::pair<int, int>, int>> q;
        std::set<std::pair<int, int>> visited_nodes;
        std::map<std::pair<int, int>, std::pair<int, int>> parent_map;

        bool start_node_walkable = true;
        if (start_node.first < 0 || start_node.first >= board.GetRows() || start_node.second < 0 || start_node.second >= board.GetCols()) {
            start_node_walkable = false;
        } else {
            int cell_type = board.GetCellType(start_node.first, start_node.second);
            switch(cell_type) {
                case 1: start_node_walkable = false; break;
                case 2: start_node_walkable = (current_game_turn % 2 == 0); break;
                case 3: {
                    start_node_walkable = false;
                    bool found_in_dw_list = false;
                    for(const auto& dw : dynamic_wall_states) {
                        if (dw.row == start_node.first && dw.col == start_node.second) {
                            found_in_dw_list = true;
                            if (dw.current_turns > 0) start_node_walkable = (dw.current_turns <= 0);
                            else start_node_walkable = true;
                            break;
                        }
                    }
                    if(cell_type==3 && !found_in_dw_list) start_node_walkable = false;
                    break;
                }
                default: break;
            }
        }
        if (!start_node_walkable) return {};

        q.push({start_node, 0});
        visited_nodes.insert(start_node);

        bool path_found = false;
        if (start_node == goal_node && start_node_walkable) {
            path_found = true;
        }

        while (!q.empty() && !path_found)
        {
            std::pair<int, int> current_pos = q.front().first;
            int turns_on_path = q.front().second;
            q.pop();

            int turn_for_neighbor_arrival_eval = current_game_turn + turns_on_path + 1;

            std::vector<std::pair<int, int>> neighbors = GetNeighbors(
                board, current_pos, turn_for_neighbor_arrival_eval,
                dynamic_wall_states, current_game_turn,
                true // is_pentagonal flag -> true
            );

            for (const auto& neighbor_pos : neighbors)
            {
                if (neighbor_pos == goal_node) {
                    parent_map[neighbor_pos] = current_pos;
                    path_found = true;
                    break;
                }

                if (visited_nodes.find(neighbor_pos) == visited_nodes.end())
                {
                    visited_nodes.insert(neighbor_pos);
                    parent_map[neighbor_pos] = current_pos;
                    q.push({neighbor_pos, turns_on_path + 1});
                }
            }
        }

        if (path_found)
        {
            return ReconstructPath(parent_map, start_node, goal_node);
        }
        return {};
    }

}
