#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>

namespace game {

class Game {
public:
    Game () {}

    virtual void step(SDL_Renderer *renderer, SDL_Event *event) {}

};

} // namespace game