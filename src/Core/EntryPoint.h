// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/Application.h"

// Third party
#include <SDL.h>

// System
#include <memory>

// Forward Declarations
extern std::unique_ptr<Application> CreateApplication();  // Defined in the client

//------------------------------------------------------------------------------
static bool InitializeSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    std::cout << "SDL initialized successfully." << std::endl;
    return true;
}

//------------------------------------------------------------------------------
int32_t SDL_main(int argc, char* argv[])
{
    (void)argc; // Avoid unused parameter warning
    (void)argv;

    if (!InitializeSDL())
    {
        return -1;
    }

	auto app = CreateApplication();
    if (!app->IsValid())
    {
        SDL_Quit();
        return -1;
    }

    app->Run();
    SDL_Quit();
    return 0;
}