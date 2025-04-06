#include "game/game.hpp"
#include <SDL2/SDL.h>

class RectangleGame : public game::Game {
public:
    RectangleGame() {}

    void step(SDL_Renderer *renderer, SDL_Event *event) {
        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw a rectangle
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect rect = {100, 100, 100, 100};
        SDL_RenderFillRect(renderer, &rect); 
    }

};