#include <iostream>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>
#include <string>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const char *bmpPath = "assets/hello_world.bmp";
const char *backgroundImagePath = "assets/background-panel.png";
SDL_Window *gWindow = NULL;

// The surface contained by the window
SDL_Surface *gScreenSurface = NULL;

// The image we will load and show on the screen
SDL_Surface *gHelloWorld = NULL;

void sleepForXSeconds(int milliSeconds);

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
        return success;
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cout << "SDL_IMAGE init Error :" << IMG_GetError() << "\n";
        success = false;
        return success;
    }

    // Video initialized , need to create window
    gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL)
    {
        std::cout << "Window could not be created! SDL_Error:" << IMG_GetError() << "\n";
        success = false;
    }
    else
    {
        // Get window surface
        gScreenSurface = SDL_GetWindowSurface(gWindow);
    }

    return success;
}

bool loadMedia()
{
    // loading success flag
    bool success = true;
    // load image

    gHelloWorld = IMG_Load(backgroundImagePath);
    if (gHelloWorld == NULL)
    {
        std::cout << "Unable to load image as is , SDL Error" << SDL_GetError() << "\n";
        success = false;
    }
    return success;
}

void close()
{
    IMG_Quit();
    // Basically a manual flush
    SDL_FreeSurface(gHelloWorld);
    gHelloWorld = NULL;

    // Destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}

void sleepForXSeconds(int milliSeconds)
{
    std::chrono::milliseconds pauseTime(milliSeconds);
    std::this_thread::sleep_for(pauseTime);
    return;
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
            bool quit = false;
            // SDL event handler ( any events will be stored here )
            SDL_Event e;
            while (!quit)
            {
                while (SDL_PollEvent(&e) != 0)
                {
                    if (e.type == SDL_QUIT)
                        quit = true;
                }
            }
            // SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, NULL);
            SDL_UpperBlitScaled(gHelloWorld, NULL, gScreenSurface, NULL);
            SDL_UpdateWindowSurface(gWindow);
            sleepForXSeconds(30000);
        }
    }

    // close before exiting the main function
    close();
    return 0;
}
