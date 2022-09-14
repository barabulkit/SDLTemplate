#include "SDL.h"
#include <stdio.h>

SDL_Window* window;
SDL_Renderer* renderer;

int main(int argc, char* argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) >= 0) {
        window = SDL_CreateWindow("SDLTest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

        if(window != 0) {
            renderer = SDL_CreateRenderer(window, -1, 0);
        }
    } else {
        printf("failed to init sdl: %s", SDL_GetError());
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
    SDL_Delay(5000);

    SDL_Quit();

    return 0;
}