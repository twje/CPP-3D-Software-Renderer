#include "Core/Application.h"

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppConfig.h"

//------------------------------------------------------------------------------
Application::Application(const AppConfig& config)
    : mContext(config)
    , mRunning(IsValid())
{ }

//------------------------------------------------------------------------------
void Application::Run()
{
    if (!IsValid())
    {
        return;
    }

    SDL_Renderer* renderer = mContext.mRenderer.GetSDLRenderer();

    OnCreate();

	const int32_t targetFrameTime = 1000 / 30;
    uint32_t previousFrameTime = SDL_GetTicks();

    while (mRunning)
    {
        ProcessEvents();
        OnUpdate();

        // Wait some time until the reach the target frame time in milliseconds
        int32_t timeToWait = targetFrameTime - (SDL_GetTicks() - previousFrameTime);

        // Only delay execution if we are running too fast
        if (timeToWait > 0)
        {
            SDL_Delay(static_cast<uint32_t>(timeToWait));
        }
        previousFrameTime = SDL_GetTicks();        

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        OnRender();
        SDL_RenderPresent(renderer);
    }
}

//------------------------------------------------------------------------------
void Application::ProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            mRunning = false;
            return;
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        {
            mRunning = false;
            return;
        }

        OnEvent(event);  // Delegate to virtual method
    }
}