#include "input_handler.h"

InputHandler::InputHandler() {
    action_toggle_path_pressed = false;
    action_restart_pressed = false;
    current_move_action = PlayerAction::NONE;
    // Key mappings are initialized in the header
}

void InputHandler::Update() {
    // Reset frame-specific states
    action_toggle_path_pressed = false;
    action_restart_pressed = false;
    current_move_action = PlayerAction::NONE; // Default to no movement action

    // Check for game actions first
    if (IsKeyPressed(key_toggle_path)) {
        action_toggle_path_pressed = true;
    }
    if (IsKeyPressed(key_restart)) {
        action_restart_pressed = true;
    }

    // Check for pentagonal movement actions (Q, W, E, A, D)
    // The order of these checks determines priority if multiple keys are pressed simultaneously.
    // Typically, IsKeyPressed handles one key press per frame correctly for distinct actions.
    if (IsKeyPressed(key_pent_up_left)) {
        current_move_action = PlayerAction::MOVE_PENT_UP_LEFT;
    } else if (IsKeyPressed(key_pent_up)) {
        current_move_action = PlayerAction::MOVE_PENT_UP;
    } else if (IsKeyPressed(key_pent_up_right)) {
        current_move_action = PlayerAction::MOVE_PENT_UP_RIGHT;
    } else if (IsKeyPressed(key_pent_left)) {
        current_move_action = PlayerAction::MOVE_PENT_LEFT;
    } else if (IsKeyPressed(key_pent_right)) {
        current_move_action = PlayerAction::MOVE_PENT_RIGHT;
    }
    // If no pentagonal keys, check for cardinal (useful if we want to support both or for menus)
    // For the game, pentagonal keys should take precedence if that's the active mode.
    // This part can be enabled if cardinal input is also desired for some reason.
    /*
    else if (IsKeyPressed(key_cardinal_up)) {
        current_move_action = PlayerAction::MOVE_UP;
    } else if (IsKeyPressed(key_cardinal_down)) {
        current_move_action = PlayerAction::MOVE_DOWN;
    } else if (IsKeyPressed(key_cardinal_left)) {
        current_move_action = PlayerAction::MOVE_LEFT;
    } else if (IsKeyPressed(key_cardinal_right)) {
        current_move_action = PlayerAction::MOVE_RIGHT;
    }
    */
}

bool InputHandler::IsActionPressed(PlayerAction action) const {
    // This function can be used to check if a specific action (including movement)
    // was triggered in the current frame.
    if (action >= PlayerAction::MOVE_PENT_UP_LEFT && action <= PlayerAction::MOVE_RIGHT) { // Covers all move types
        return current_move_action == action;
    }
    switch (action) {
        case PlayerAction::TOGGLE_PATH:
            return action_toggle_path_pressed;
        case PlayerAction::RESTART_GAME:
            return action_restart_pressed;
        default:
            return false; // Should not happen if all actions are covered
    }
}

bool InputHandler::IsTogglePathPressed() const {
    return action_toggle_path_pressed;
}

bool InputHandler::IsRestartPressed() const {
    return action_restart_pressed;
}

PlayerAction InputHandler::GetMoveAction() const {
    // Returns the single movement action detected for this frame.
    // Game logic will use this to decide how to move the player.
    return current_move_action;
}

// Example usage in Game class (conceptual):
// void Game::ProcessInputs(const InputHandler& input) {
//     PlayerAction move = input.GetMoveAction();
//     if (move != PlayerAction::NONE) {
//         HandlePlayerMovement(move); // This new function will use the pentagonal logic
//     }
//     if (input.IsTogglePathPressed()) {
//         show_bfs_path = !show_bfs_path;
//     }
//     // ... etc.
// }
