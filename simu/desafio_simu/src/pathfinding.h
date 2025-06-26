#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <vector>
#include <queue>
#include <map>
#include <utility> // For std::pair
#include "board.h" // Needs to know about Board to check walkable cells
#include "game.h"  // Needs DynamicWall definition for IsCellWalkable context

// Using MazeGrid from board.h
// Using DynamicWall from game.h

namespace Pathfinding
{
    // BFS function
    // Takes the board, start/goal coordinates, dynamic wall states, and current turn.
    // Returns a vector of coordinates representing the path from start to goal.
    // The path includes the start and goal nodes. Returns empty if no path found.
    std::vector<std::pair<int, int>> FindShortestPathBFS(
        const Board& board,
        std::pair<int, int> start_node,
        std::pair<int, int> goal_node,
        const std::vector<DynamicWall>& dynamic_wall_states, // Current state of dynamic walls
        int current_turn                                     // Current turn for evaluating turn-based cells
    );

    // A* function (to be implemented later)
    // std::vector<std::pair<int, int>> FindShortestPathAStar(
    //     const Board& board,
    //     std::pair<int, int> start_node,
    //     std::pair<int, int> goal_node,
    //     const std::vector<DynamicWall>& dynamic_wall_states,
    //     int current_turn
    // );

    // Helper for BFS/A* to get valid neighbors.
    // This will be crucial for pentagonal grid logic.
    std::vector<std::pair<int, int>> GetNeighbors(
        const Board& board,
        std::pair<int, int> current_node,
        int turn_for_eval, // The absolute game turn at which the neighbor would be entered
        const std::vector<DynamicWall>& dynamic_wall_states, // Live state of walls when BFS started
        int bfs_start_turn, // The game turn when BFS was initiated
        bool is_pentagonal // Flag to switch neighbor logic
    );

} // namespace Pathfinding

#endif // PATHFINDING_H
