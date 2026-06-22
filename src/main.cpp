#include <iostream>
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    [[maybe_unused]] SDL_Window *window = NULL;
    [[maybe_unused]] SDL_Surface *screenSurface = NULL;
    // Initalize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL Could not initialize! SDL_ERROR" << SDL_GetError() << "\n";
    }
    else
    {
        // Create a window

        window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            std::cout << "Window could not be created SDL_ERROR" << SDL_GetError();
        }
        else
        {
            // Get window surface
            screenSurface = SDL_GetWindowSurface(window);
            // Fill the surface white
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
            // Update teh surface
            SDL_UpdateWindowSurface(window);

            // Hack to get eindow to stay up
            SDL_Event e;
            bool quit = false;
            while (quit == false)
            {
                while (SDL_PollEvent(&e))
                {
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                }
            }
        }
    }
    SDL_Quit();
    return 0;
}
