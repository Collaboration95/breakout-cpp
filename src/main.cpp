#include <iostream>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <string>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const char *bmpPath = "assets/hello_world.bmp";
SDL_Window *gWindow = NULL;

// The surface contained by the window
SDL_Surface *gScreenSurface = NULL;

// The image we will load and show on the screen
SDL_Surface *gHelloWorld = NULL;

// function prototypes for 3 main functions
bool init();
bool loadMedia();
void close();

bool init()
{
    bool success = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL Could not initialize due to  SDL_ERROR : " << SDL_GetError() << "\n";
        success = false;
    }
    else
    {
        // Video initialized , need to create window
        gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            std::cout << "Window could not be created! SDL_Error:" << SDL_GetError() << "\n";
            success = false;
        }
        else
        {
            // Get window surface
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }
    return success;
}

bool loadMedia()
{
    // loading success flag
    bool success = true;
    // load image

    gHelloWorld = SDL_LoadBMP(bmpPath);
    if (gHelloWorld == NULL)
    {
        std::cout << "Unable to load image as is , SDL Error" << SDL_GetError() << "\n";
        success = false;
    }
    return success;
}

void close()
{
    // Basically a manual flush
    SDL_FreeSurface(gHelloWorld);
    gHelloWorld = NULL;

    // Destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}
int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    if (!init())
    {
        std::cout << "Failed to initilaize \n";
    }
    else
    {
        if (!loadMedia())
        {
            std::cout << "Failed to load media \n";
        }
        else
        {
            SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, NULL);
            SDL_UpdateWindowSurface(gWindow);
            SDL_Event e;
            bool quit = false;
            while (quit == false)
            {
                while (SDL_PollEvent(&e))
                {
                    if (e.type == SDL_QUIT)
                        quit = true;
                }
            }
        }
    }

    // close before exiting the main function
    close();
    return 0;
}
