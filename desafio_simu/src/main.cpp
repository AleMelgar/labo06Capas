#include "game.h"
#include "config.h" // Podría ser necesario para alguna configuración inicial si no se maneja en Game
#include <iostream> // Para std::cerr en caso de error de inicialización de Game

int main() {
    const std::string maze_filename = "maze.txt";

    // Se podría añadir un try-catch si el constructor de Game pudiera lanzar excepciones.
    Game escape_the_grid_game(maze_filename);


    escape_the_grid_game.Run();

    // CloseWindow() es llamado por el destructor de Renderer, que es llamado por el destructor de Game.
    return 0;
}
