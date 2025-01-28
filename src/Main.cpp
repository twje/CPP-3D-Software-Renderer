#include "pch.h"

// Includes
//------------------------------------------------------------------------------
// Third party
#include <SDL.h>

//------------------------------------------------------------------------------
int SDL_main(int argc, char* argv[]) 
{
    (void)argc; // Avoid unused parameter warning
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    // SDL application logic here...

    SDL_Quit();
    return 0;
}