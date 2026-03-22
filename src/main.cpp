#include "Game.h"
#include <SDL.h>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Game game;

    if (!game.init()) {
        SDL_Log("Failed to initialize game");
        return 1;
    }

    game.run();
    game.shutdown();

    return 0;
}
