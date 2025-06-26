#include "game.h"
#include "board.h" // Included via game.h but good for clarity
#include "input_handler.h" // Included via game.h
#include <iostream> // For std::cerr (debugging)

// Constructor
Game::Game(const Board& board_ref)
    : game_board(board_ref) // Initialize reference member
{
    InitializeGameMembers(board_ref);
}

Game::~Game()
{
    // Cleanup if needed (e.g., if any dynamic memory was allocated by Game itself)
}

void Game::InitializeGameMembers(const Board& board_ref) {
    player.row = board_ref.GetPlayerStart().first;
    player.col = board_ref.GetPlayerStart().second;
    player.movement_history.clear();
    player.movement_history.push_back({player.row, player.col});
    player.active = true;

    clone.row = -1; // Off-board initially
    clone.col = -1;
    clone.movement_history.clear(); // Not strictly needed for clone as it follows player's
    clone.active = false;

    current_turn = 0;
    current_game_state = GameState::PLAYING;
    player_moved_this_turn = false; // Reset flag

    // Initialize live_dynamic_walls from the board's configuration
    live_dynamic_walls.clear();
    const auto& wall_configs = board_ref.GetDynamicWallsConfig();
    for (const auto& config : wall_configs) {
        // In Board, DynamicWall stores initial config. Here, we make a live copy.
        // Assuming Board's DynamicWall struct is {row, col, initial_turns, initial_turns}
        // And Game's DynamicWall struct is {row, col, initial_turns, current_countdown}
        live_dynamic_walls.push_back({config.row, config.col, config.initial_turns_to_open, config.initial_turns_to_open});
    }

    // Validate player start position against the actual grid from board_ref
    if (player.row < 0 || player.row >= game_board.GetRows() ||
        player.col < 0 || player.col >= game_board.GetCols() ||
        game_board.GetCellType(player.row, player.col) == 1) // 1 is wall
    {
        TraceLog(LOG_WARNING, "Player start position (%d,%d) is invalid. Attempting to find a new start.", player.row, player.col);
        bool found_start = false;
        for (int r = 0; r < game_board.GetRows(); ++r) {
            for (int c = 0; c < game_board.GetCols(); ++c) {
                if (game_board.GetCellType(r, c) == 0) { // 0 is path
                    player.row = r;
                    player.col = c;
                    player.movement_history.clear();
                    player.movement_history.push_back({player.row, player.col});
                    found_start = true;
                    TraceLog(LOG_INFO, "New player start position found: (%d,%d)", player.row, player.col);
                    break;
                }
            }
            if (found_start) break;
        }
        if (!found_start) {
            TraceLog(LOG_ERROR, "No valid starting position found for player. Game cannot start.");
            current_game_state = GameState::GAME_OVER_LOSE;
        }
    }
}

void Game::Restart(const Board& board_ref) {
    // game_board reference is const, cannot reassign.
    // If board can change, Game should take a pointer or use a different mechanism.
    // For now, InitializeGameMembers will re-initialize based on the *same* board_ref
    // or a new one if the design allows Game to be reconstructed or board_ref updated.
    // Assuming board_ref passed to Restart is the one to use now.
    // This implies Game needs to be able to change its board_ref or the design changes.
    // Simplest for now: re-initialize with the initially provided board.
    // If you need to load a *new* maze file and restart, main.cpp should create a new Board and new Game.
    // For a simple Restart button, we reset state with the current board.
    InitializeGameMembers(this->game_board); // Re-initialize with the existing board
}


void Game::Update()
{
    if (current_game_state != GameState::PLAYING) {
        return;
    }
    // Player movement is now initiated by ProcessPlayerMove through main loop's input handling.
    // If player_moved_this_turn is true, then HandlePlayerMovement would have already:
    // - Incremented current_turn
    // - Updated dynamic walls
    // - Updated clone
    // - Checked end conditions
    // So, this Update function might become simpler, or mainly for non-player triggered updates if any.
    // For now, assuming HandlePlayerMovement did all necessary post-move updates.
    // One thing to ensure is clone collision is checked even if player doesn't move but clone does.
    if (clone.active) { // Check clone collision regardless of player move
        CheckCloneCollision();
    }
}

void Game::ProcessPlayerMove(PlayerAction action) {
    if (current_game_state != GameState::PLAYING) return;

    int dr = 0, dc = 0;
    bool attempt_move = true;

    // Determine (dr, dc) based on PlayerAction and player.row parity for pentagonal moves
    // For Q, W, E, A, D
    // Q: Up-Left, W: Up, E: Up-Right, A: Left, D: Right (conceptual)
    // Actual (dr,dc) depends on row parity for vertical connections.
    bool is_even_row = (player.row % 2 == 0);

    switch (action) {
        // Pentagonal Moves - Relative to player's current cell (player.row, player.col)
        // These (dr,dc) are examples and need to match your chosen pentagonal grid logic.
        // Assuming:
        // Even Rows (0,2,..): Point-Up pentagons (flat base down). Neighbors: UL, U, UR, L, R (more like hex connection)
        // Odd Rows (1,3,..): Point-Down pentagons (flat base up), shifted.
        // Let's use the 5-direction scheme discussed:
        // If FILA PAR (player.row % 2 == 0): (Pentágono "apuntando arriba" o base plana abajo)
        //   1. Arriba-Izquierda (Q ~ MOVE_PENT_UP_LEFT): (r-1, c-1)
        //   2. Arriba-Derecha (W ~ MOVE_PENT_UP):      (r-1, c)  <-- This would be Up for a 'point-up' hex, let's adjust
        //   Let's map QWEAD to 5 distinct directions.
        //   Q: Top-Left, W: Top-Center/Up, E: Top-Right, A: Left, D: Right
        //   If even row (e.g. point up):
        //      L: (0,-1), R: (0,1)
        //      Connects downwards to point of odd row: (1,c) -> W or S
        //      Connects upwards-diag to flat of odd row: (-1, c-1), (-1,c) -> Q, E
        //
        // Let's use the definition from the plan:
        // FILA PAR (r, c): (Pentágono "apuntando arriba")
        //  Q (UL): (r-1, c-1)  (Key Q for "Top-Left-Ish")
        //  W (UR): (r-1, c)    (Key W for "Top-Right-Ish")
        //  A (L):  (r, c-1)
        //  S (D):  (r+1, c)    (Key S for "Down" - needs a key, let's map E to this for now)
        //  D (R):  (r, c+1)
        //  This is 5 keys. Q,W,A,S,D. You chose Q,W,E,A,D.
        //  Let's map: Q=UL, W=U/CenterTop, E=UR, A=L, D=R

        case PlayerAction::MOVE_PENT_LEFT: // A
            dr = 0; dc = -1;
            break;
        case PlayerAction::MOVE_PENT_RIGHT: // D
            dr = 0; dc = 1;
            break;

        // Vertical/Diagonal movements depend on row parity
        case PlayerAction::MOVE_PENT_UP_LEFT: // Q
            if (is_even_row) { dr = -1; dc = -1; } // Point up: connects to base-left of pentagon above (odd row)
            else { dr = -1; dc = 0; }             // Point down: connects to tip of pentagon above (even row)
            break;
        case PlayerAction::MOVE_PENT_UP: // W
            if (is_even_row) { dr = -1; dc = 0; }  // Point up: connects to base-right of pentagon above (odd row)
            else { dr = -1; dc = 1; }              // Point down: connects to tip of pentagon above (even row), shifted right
            break;
        case PlayerAction::MOVE_PENT_UP_RIGHT: // E
            if (is_even_row) { dr = 1; dc = 0; }   // Point up: connects to tip of pentagon below (odd row)
            else { dr = 1; dc = 0; }               // Point down: connects to base-left of pentagon below (even row)
            // This mapping for E needs to be one of the 5 unique.
            // The previous plan was:
            // PAR (r,c): Q(r-1,c-1), W(r-1,c), A(r,c-1), D(r,c+1), plus one S(r+1,c)
            // IMPAR (r,c): Q(r-1,c), W(r,c-1), A(r,c+1), D(r+1,c), plus one S(r+1,c+1)
            // This requires careful re-mapping of QWEAD to these 5 distinct (dr,dc) sets per parity.
            // Let's redefine based on Q,W,E,A,D keys and desired intuitive mapping.
            //
            // Revised mapping based on typical QWEAD usage for directions:
            // A = Left, D = Right (these are consistent)
            // W = "Forward/Up", Q = "Forward-Left/Up-Left", E = "Forward-Right/Up-Right"
            // Row Parity determines the exact (dr, dc) for Q, W, E.
            //
            // If player.row is EVEN: (pentagon has flat base at bottom, point up)
            //   A: (0, -1)                     [Left]
            //   D: (0, +1)                     [Right]
            //   W: (-1, player.col % 2 == 0 ? 0 : -1)  -- This needs to be simpler.
            //   Let's use your original proposal more directly:
            //   FILA PAR (r, c) (e.g. point up):
            //     MOVE_PENT_LEFT (A):      dr=0,  dc=-1
            //     MOVE_PENT_RIGHT (D):     dr=0,  dc=1
            //     MOVE_PENT_UP_LEFT (Q):   dr=-1, dc=-1  (Connects to r-1, c-1)
            //     MOVE_PENT_UP (W):        dr=-1, dc=0   (Connects to r-1, c)
            //     MOVE_PENT_UP_RIGHT (E):  dr=1,  dc=0   (This was S in your plan for Par, (r+1,c). Let's use E for a 5th unique)
                                                        // If E is (r+1,c) for PAR, this is "Down"
            if (is_even_row) { // Player on an even row (e.g., 0, 2...)
                switch (action) {
                    case PlayerAction::MOVE_PENT_LEFT:     dr = 0; dc = -1; break; // A
                    case PlayerAction::MOVE_PENT_RIGHT:    dr = 0; dc = +1; break; // D
                    case PlayerAction::MOVE_PENT_UP_LEFT:  dr = -1; dc = -1; break; // Q
                    case PlayerAction::MOVE_PENT_UP:       dr = -1; dc = 0;  break; // W
                    case PlayerAction::MOVE_PENT_UP_RIGHT: dr = +1; dc = 0;  break; // E (mapped to "Down" for even rows)
                    default: attempt_move = false; break;
                }
            } else { // Player on an odd row (e.g., 1, 3...)
                // FILA IMPAR (r, c) (e.g. point down):
                //   MOVE_PENT_LEFT (A):      dr=0,  dc=-1
                //   MOVE_PENT_RIGHT (D):     dr=0,  dc=1
                //   MOVE_PENT_UP_LEFT (Q):   dr=-1, dc=0    (Connects to r-1, c)
                //   MOVE_PENT_UP (W):        dr=1,  dc=0    (Connects to r+1, c) <-- This was SW (r+1,c) in plan. Let W be "Down-Left"
                //   MOVE_PENT_UP_RIGHT (E):  dr=1,  dc=1    (Connects to r+1, c+1) <-- This was SE (r+1,c+1) in plan. Let E be "Down-Right"
                //   This means Q is Up, W is Down-Left, E is Down-Right for ODD rows.
                 switch (action) {
                    case PlayerAction::MOVE_PENT_LEFT:     dr = 0; dc = -1; break; // A
                    case PlayerAction::MOVE_PENT_RIGHT:    dr = 0; dc = +1; break; // D
                    case PlayerAction::MOVE_PENT_UP_LEFT:  dr = -1; dc = 0;  break; // Q (mapped to "Up" for odd rows)
                    case PlayerAction::MOVE_PENT_UP:       dr = +1; dc = 0;  break; // W (mapped to "Down-Left" for odd rows)
                    case PlayerAction::MOVE_PENT_UP_RIGHT: dr = +1; dc = +1; break; // E (mapped to "Down-Right" for odd rows)
                    default: attempt_move = false; break;
                }
            }
            break; // End of combined switch for Q,W,E

        // Fallback for cardinal moves if they were still active (e.g. from old input handler)
        // case PlayerAction::MOVE_UP:    dr = -1; dc = 0; break;
        // case PlayerAction::MOVE_DOWN:  dr = 1;  dc = 0; break;
        // case PlayerAction::MOVE_LEFT:  dr = 0;  dc = -1; break;
        // case PlayerAction::MOVE_RIGHT: dr = 0;  dc = 1; break;
        default:
            attempt_move = false;
            break;
    }

    if (attempt_move) {
        HandlePlayerMovement(dr, dc);
    }
}


void Game::HandlePlayerMovement(int dr, int dc) {
    // This function assumes dr, dc are already determined by ProcessPlayerMove
    if (current_game_state != GameState::PLAYING) return;

    int next_row = player.row + dr;
    int next_col = player.col + dc;

    // Boundary checks using game_board
    if (next_row < 0 || next_row >= game_board.GetRows() || next_col < 0 || next_col >= game_board.GetCols()) {
        return; // Invalid move: out of bounds
    }

    // Cell collision checks
    int cell_type = game_board.GetCellType(next_row, next_col); // Base type from board
    bool cell_is_blocked = false;

    if (cell_type == 1) { // Standard wall
        cell_is_blocked = true;
    } else if (cell_type == 2) { // Even/Odd turn wall
        if (current_turn % 2 != 0) { // Blocked on odd turns (example, can be reversed)
            cell_is_blocked = true;
        }
    } else if (cell_type == 3) { // Dynamic wall that opens after N turns
        for (const auto& dw : live_dynamic_walls) {
            if (dw.row == next_row && dw.col == next_col && dw.current_turns > 0) {
                cell_is_blocked = true;
                break;
            }
        }
    }

    // Clone collision check
    if (clone.active && next_row == clone.row && next_col == clone.col) {
        cell_is_blocked = true;
        current_game_state = GameState::GAME_OVER_LOSE; // Player moved into clone - Game Over
        // No need to return yet, let the blocked move prevent actual position update,
        // then the game state will be handled by the main loop.
        // Or, could set player_moved_this_turn = false and return, but current flow is ok.
    }

    if (!cell_is_blocked) {
        // If game ended due to player moving into clone, don't update player position further.
        if (current_game_state == GameState::GAME_OVER_LOSE && player.row == clone.row && player.col == clone.col) {
             player_moved_this_turn = false; // Player attempted a move that resulted in game over
             // No actual movement or turn increment if game over by this action.
        } else {
            player.row = next_row;
        player.col = next_col;
        player.movement_history.push_back({player.row, player.col});

        current_turn++;
        player_moved_this_turn = true; // Set flag

        UpdateDynamicWalls();
        UpdateClone();
        CheckEndConditions();
        if(current_game_state == GameState::PLAYING) CheckCloneCollision(); // Check again after clone might have moved
    } else {
        player_moved_this_turn = false;
    }
}

void Game::UpdateDynamicWalls() {
    for (auto &dw : live_dynamic_walls) {
        if (dw.current_turns > 0) {
            dw.current_turns--;
            // No need to change game_board's grid; rendering and pathfinding use live_dynamic_walls
        }
    }
}

void Game::UpdateClone() {
    if (!clone.active && current_turn >= CLONE_ACTIVATION_TURNS) {
        clone.active = true;
        if (!player.movement_history.empty()) {
            // Clone starts at player's initial position recorded at the turn it activates.
            // History index 0 is player's state at turn 0.
            clone.row = player.movement_history[0].first;
            clone.col = player.movement_history[0].second;
        } else { // Should not happen if history is recorded from start
            clone.row = game_board.GetPlayerStart().first;
            clone.col = game_board.GetPlayerStart().second;
        }
    }

    if (clone.active) {
        // Clone follows player's path, delayed by CLONE_ACTIVATION_TURNS
        int history_index = current_turn - CLONE_ACTIVATION_TURNS;

        if (history_index >= 0 && history_index < player.movement_history.size()) {
            clone.row = player.movement_history[history_index].first;
            clone.col = player.movement_history[history_index].second;
            // Collision with player is checked in HandlePlayerMovement and CheckCloneCollision
        }
    }
}

void Game::CheckCloneCollision() {
    if (clone.active && player.row == clone.row && player.col == clone.col) {
        current_game_state = GameState::GAME_OVER_LOSE;
    }
}


void Game::CheckEndConditions() {
    if (current_game_state != GameState::PLAYING) return;

    // Win condition: Player reaches the goal cell
    std::pair<int,int> goal_pos = game_board.GetGoalPosition();
    if (player.row == goal_pos.first && player.col == goal_pos.second) {
        current_game_state = GameState::GAME_OVER_WIN;
    }
    // Lose condition (clone collision) is handled by CheckCloneCollision
    // and also preemptively in HandlePlayerMovement if player tries to move into clone.
}


// --- Getters ---
GameState Game::GetCurrentState() const { return current_game_state; }
int Game::GetCurrentTurn() const { return current_turn; }
const Player& Game::GetPlayer() const { return player; }
const Player& Game::GetClone() const { return clone; }
std::pair<int, int> Game::GetPlayerPosition() const { return {player.row, player.col}; }
const std::vector<DynamicWall>& Game::GetDynamicWallStates() const { return live_dynamic_walls; }
bool Game::HasPlayerMovedThisTurn() const { return player_moved_this_turn; }
void Game::ResetPlayerMovedFlag() { player_moved_this_turn = false; }


// The old LoadMazeFromFile_Internal and other direct maze manipulations are removed
// as Game class now relies on the Board object.
// GetRows, GetCols, GetCellType are also removed as they should be called on the game_board reference.
// IsDynamicWallActive is effectively replaced by iterating live_dynamic_walls in HandlePlayerMovement.
