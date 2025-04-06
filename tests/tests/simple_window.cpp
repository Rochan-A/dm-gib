#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <memory>

#include "error_reporter/error_reporter.hpp"
#include "game/game.hpp"
#include "tests/rectangle.hpp"

int main() {
    if (SDL_Init(SDL_INIT_EVERYTHING) > 0){
        REPORT_ERROR("Failed to init SDL");
    }

    static SDL_Window *window = NULL;
    static SDL_Renderer *renderer = NULL;
    window = SDL_CreateWindow("Dummy Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        REPORT_ERROR("Failed to create SDL window");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        REPORT_ERROR("Failed to create SDL renderer");
    }

    std::unique_ptr<game::Game> game = std::make_unique<RectangleGame>();

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        game->step(renderer, &event);

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    // Cleanup and destroy
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
