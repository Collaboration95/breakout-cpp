#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// ── Constants ────────────────────────────────────────────────────────────────
const int SCREEN_WIDTH = 864;
const int SCREEN_HEIGHT = 558;
const int PADDLE_BOUNDARY_LEFT = 80;
const int PADDLE_BOUNDARY_RIGHT = SCREEN_WIDTH - PADDLE_BOUNDARY_LEFT;

const char *backgroundImagePath = "assets/background-panel.png";

// ── Owned SDL lifetime (BRK-003 Req 1: one owner, one release path) ─────────
// Each native resource has exactly one RAII owner. No globals, no manual close().

// Instantiate a Keypress enum to handle left / right keypress
enum PaddleKeyPress
{
    KEYPRESS_LEFT,
    KEYPRESS_RIGHT,
};

struct Paddle
{
    int w = 120;
    int h = 40;
    int x = (int)(SCREEN_WIDTH * .50) - (int)(w * .5);
    int y = (int)(SCREEN_HEIGHT * .83) - (int)(h * .5);

private:
    int paddleSpeed = 10;
    int fastFactor = 2;

public:
    void move(int dir, bool isFast = false)
    {
        if (isFast)
        {
            x += dir * fastFactor;
        }
        else
        {
            x += dir * fastFactor * 2;
        }
    }
};
struct SdlContext
{
    SdlContext()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
        }
    }
    ~SdlContext() { SDL_Quit(); }

    /*Preventing assignment to other variables, to avid double scope exits */

    SdlContext(const SdlContext &) = delete;
    SdlContext &operator=(const SdlContext &) = delete;
    SdlContext(SdlContext &&) = delete;
    SdlContext &operator=(SdlContext &&) = delete;
};

struct SdlImageContext
{
    explicit SdlImageContext(int flags)
    {
        if ((IMG_Init(flags) & flags) != flags)
        {
            throw std::runtime_error(std::string("IMG_Init failed: ") + IMG_GetError());
        }
    }
    ~SdlImageContext() { IMG_Quit(); }

    SdlImageContext(const SdlImageContext &) = delete;
    SdlImageContext &operator=(const SdlImageContext &) = delete;
    SdlImageContext(SdlImageContext &&) = delete;
    SdlImageContext &operator=(SdlImageContext &&) = delete;
};

using WindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>;
using RendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    try
    {
        // Acquire in dependency order — destruction is reverse (Req 2).
        SdlContext sdl;
        SdlImageContext sdlImage(IMG_INIT_PNG | IMG_INIT_JPG);

        WindowPtr window{
            SDL_CreateWindow("Breakout CPP", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN),

            &SDL_DestroyWindow};

        if (!window)
        {
            throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
        }

        RendererPtr renderer{
            SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
            &SDL_DestroyRenderer};

        if (!renderer)
        {
            throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
        }

        SurfacePtr helloWorld{IMG_Load(backgroundImagePath), &SDL_FreeSurface};

        if (!helloWorld)
        {
            throw std::runtime_error(std::string("IMG_Load failed: ") + IMG_GetError() + " / " +
                                     SDL_GetError());
        }
        TexturePtr bg{SDL_CreateTextureFromSurface(renderer.get(), helloWorld.get()), &SDL_DestroyTexture};

        if (!bg)
        {
            throw std::runtime_error(std::string("SDL_CreateTextureFromSurface Failed") + SDL_GetError());
        }
        Paddle paddle;
        SDL_Rect probe{paddle.x, paddle.y, paddle.w, paddle.h};
        // ── BRK-002 loop (unchanged behavior) ────────────────────────────────
        bool quit = false;
        int dx = 10; // var to determine left/ right paddle movement speed
        SDL_Event e;
        while (!quit)
        {
            while (SDL_PollEvent(&e) != 0)
            {
                switch (e.type)
                {
                case SDL_QUIT:
                    quit = true;
                    std::cout << "Quit Entered\n";
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        quit = true;
                        std::cout << "Escape Entered\n";

                        break;
                    case SDLK_RIGHT:

                        paddle.move(1);
                        probe.x = paddle.x;

                        std::cout << "RIGHTKEYPRESS" << "\n";
                        break;
                    case SDLK_LEFT:

                        paddle.move(-1);
                        probe.x = paddle.x;

                        std::cout << "LEft keypress\n";
                        break;
                    default:
                        std::cout << "Key pressed code is :" << SDL_GetKeyName(e.key.keysym.sym) << "\n";
                        break;
                    }
                    break;
                default:
                    break;
                }
            }

            // need to handle multiple keys pressed at once
            const Uint8 *ks = SDL_GetKeyboardState(nullptr);
            int dir = (ks[SDL_SCANCODE_RIGHT] ? 1 : 0) - (ks[SDL_SCANCODE_LEFT] ? 1 : 0);
            if (ks[SDL_SCANCODE_LALT])
            {
                paddle.move(dir, true);
            }
            else
            {
                paddle.move(dir);
            }

            /* Can draw rectangle here ? for now */
            /* Setting rbgA value , ie setting stuff to white ?*/
            // SDL_RenderClear(renderer.get());

            // SDL_SetRenderDrawColor(renderer.get(), 0xFF, 0xFF, 0xFF, 0xFF);
            // SDL_UpperBlitScaled(helloWorld.get(), nullptr, screenSurface, nullptr);
            SDL_SetRenderDrawColor(renderer.get(), 0x1E, 0x1E, 0x1E, 0xFF);

            if (SDL_RenderClear(renderer.get()) < 0)
            {
                std::cerr << "SDL_RenderClear failed:" << SDL_GetError() << "\n";
            }

            if (SDL_RenderCopy(renderer.get(), bg.get(), nullptr, nullptr) < 0)
            {
                std::cerr << "SDL_RenderCopy failed:" << SDL_GetError() << "\n";
            }

            SDL_SetRenderDrawColor(renderer.get(), 1, 1, 1, 0xFF);

            if (SDL_RenderFillRect(renderer.get(), &probe) < 0)
            {
                std::cerr << "SDL_RenderFillRect failed:" << SDL_GetError() << "\n";
            }
            SDL_RenderPresent(renderer.get());

            SDL_Delay(16);
        }
        // No manual close() — WindowPtr/SurfacePtr/Context destructors release in order:
        // helloWorld -> window (invalidates screenSurface) -> IMG_Quit -> SDL_Quit
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << "\n";
        // Already-acquired owners unwind here (Req 3: partial init does not leak)
        return 1;
    }
    return 0;
}
