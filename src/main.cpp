#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <chrono>
#include <thread>
#include <string>

const int SCREEN_WIDTH = 864;
const int SCREEN_HEIGHT = 558;
const char *bmpPath = "assets/hello_world.bmp";
const char *backgroundImagePath = "assets/background-panel.png";
// TODO(BRK-003): Wrap owning SDL handles in RAII — SDL_CreateWindow / SDL_GetWindowSurface / IMG_Load
// currently use raw owning globals (gWindow, gScreenSurface, gHelloWorld) + manual close().
// In BRK-003 replace with std::unique_ptr with custom deleter or a small RAII wrapper
// (e.g. WindowPtr = unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>), so close()'s
// SDL_FreeSurface/SDL_DestroyWindow/IMG_Quit/SDL_Quit vanish into destructors.
// See docs/03-cpp-port-roadmap.md Phase 0.
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
    // TODO(BRK-003): SDL_CreateWindow ownership currently raw — will move into RAII wrapper.
    gWindow = SDL_CreateWindow("Breakout CPP", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
                    switch (e.type)
                    {
                    case SDL_QUIT:
                        quit = true;
                        std::cout << "Quit Entered";
                        break;
                    case SDL_KEYDOWN:
                        switch (e.key.keysym.sym)
                        {
                        case (SDLK_ESCAPE):
                            quit = true;
                            std::cout << "Escape Entered";
                            break;
                        }
                        break;
                    }
                }
                SDL_UpperBlitScaled(gHelloWorld, NULL, gScreenSurface, NULL);
                SDL_UpdateWindowSurface(gWindow);
                SDL_Delay(16); // to avoid busy spinning
            }
            // SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, NULL);
        }
    }

    // close before exiting the main function
    close();
    return 0;
}
