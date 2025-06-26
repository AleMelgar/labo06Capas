#include "player_entity.h"
#include "config.h" // Para CLONE_ACTIVATION_TURNS

void Clone::UpdatePositionFromHistory(const MovementHistory& player_history, int current_turn) {
    if (!active) return;

    int history_idx = current_turn - CLONE_ACTIVATION_TURNS;

    if (history_idx >= 0 && history_idx < player_history.size()) {
        if (current_turn == CLONE_ACTIVATION_TURNS && !player_history.empty()) {
            pos = player_history[0];
        } else if (history_idx > 0 && history_idx < player_history.size()) {
            pos = player_history[history_idx];
        }
    }
}
