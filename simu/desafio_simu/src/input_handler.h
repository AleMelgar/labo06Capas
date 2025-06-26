#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "raylib.h" // For key codes

// Enum for different actions player can take.
enum class PlayerAction {
    NONE,

    // Pentagonal movements - abstract directions
    // Mapping to Q, W, E, A, D will be done in cpp
    // These names are conceptual based on a common layout.
    // The actual (dr, dc) will be determined by Game class based on row parity.
    MOVE_PENT_UP_LEFT,      // Q
    MOVE_PENT_UP,           // W
    MOVE_PENT_UP_RIGHT,     // E
    MOVE_PENT_LEFT,         // A
    MOVE_PENT_RIGHT,        // D
    // Note: We have 5 keys for 5 directions.
    // The interpretation of "UP_LEFT", "UP", "UP_RIGHT" might shift
    // depending on whether the pentagon is "point-up" or "point-down".
    // Game class will handle the mapping from these abstract actions to grid dx, dy.

    // Standard Cardinal (fallback or for different game modes, keep for now if any part still uses them)
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,

    TOGGLE_PATH, // For 'S' key
    RESTART_GAME, // For 'R' key (optional)
    QUIT_GAME    // For ESC key (implicitly handled by WindowShouldClose usually)
};

class InputHandler
{
public:
    InputHandler();

    void Update(); // Polls for input

    // Check if an action was triggered this frame
    bool IsActionPressed(PlayerAction action) const;
    // Check if an action key is currently held (for future use, e.g. continuous move)
    // bool IsActionDown(PlayerAction action) const;

    // Specific direct key checks (can be phased out if actions cover all needs)
    bool IsTogglePathPressed() const;
    bool IsRestartPressed() const;

    // Returns the PlayerAction corresponding to the movement key pressed this frame.
    PlayerAction GetMoveAction() const;


private:
    // State for actions triggered this frame. Cleared each Update.
    bool action_toggle_path_pressed;
    bool action_restart_pressed;
    PlayerAction current_move_action; // Stores the movement action for the current frame

    // Key mappings (could be made configurable in the future)
    // For Pentagonal Movement:
    KeyboardKey key_pent_up_left = KEY_Q;
    KeyboardKey key_pent_up      = KEY_W;
    KeyboardKey key_pent_up_right= KEY_E;
    KeyboardKey key_pent_left    = KEY_A;
    KeyboardKey key_pent_right   = KEY_D;

    // For Standard Cardinal Movement (if ever needed as primary)
    KeyboardKey key_cardinal_up    = KEY_UP;
    KeyboardKey key_cardinal_down  = KEY_DOWN;
    KeyboardKey key_cardinal_left  = KEY_LEFT;
    KeyboardKey key_cardinal_right = KEY_RIGHT;

    // For Other Actions
    KeyboardKey key_toggle_path = KEY_S;
    KeyboardKey key_restart     = KEY_R;
};

#endif // INPUT_HANDLER_H
