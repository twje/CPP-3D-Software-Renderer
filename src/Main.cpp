// Includes
//------------------------------------------------------------------------------
// Third party
#include <SDL.h>

//------------------------------------------------------------------------------
class SDLSystem
{
public:
    explicit SDLSystem(uint32_t flags = SDL_INIT_VIDEO)
        : mWindow(nullptr)
		, mRenderer(nullptr)
        , mInitialized(false)
    {
        if (!InitializeSDL(flags) || !CreateWindow() || !CreateRenderer())
        {
            return;  // Failure is logged inside helper functions
        }
    }

    ~SDLSystem()
    {
		if (mRenderer)
		{
			SDL_DestroyRenderer(mRenderer);
			std::cout << "SDL Renderer destroyed successfully." << std::endl;
		}

        if (mWindow)
        {
            SDL_DestroyWindow(mWindow);
            std::cout << "SDL Window destroyed successfully." << std::endl;
        }

        if (mInitialized)
        {
            SDL_Quit();
            std::cout << "SDL shutdown successfully." << std::endl;
        }
    }

    bool IsValid() const { return mInitialized && mWindow && mRenderer; }

    SDL_Window* GetSDLWindow() { return mWindow; }
	SDL_Renderer* GetSDLRenderer() { return mRenderer; }

    // Delete copy and assignment to prevent accidental re-initialization
    SDLSystem(const SDLSystem&) = delete;
    SDLSystem& operator=(const SDLSystem&) = delete;

private:
    bool InitializeSDL(uint32_t flags)
    {
        if (SDL_Init(flags) == 0)
        {
            mInitialized = true;
            std::cout << "SDL initialized successfully." << std::endl;
            return true;
        }

        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    bool CreateWindow()
    {
        mWindow = SDL_CreateWindow(
            "SDL Application",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800,
            600,
            SDL_WINDOW_SHOWN);

        if (mWindow)
        {
            std::cout << "SDL Window created successfully." << std::endl;
            return true;
        }

        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    bool CreateRenderer()
    {
		mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (mRenderer)
        {
			std::cout << "SDL Renderer created successfully." << std::endl;
			return true;
        }

		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
    }

    SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
    bool mInitialized;
};

//------------------------------------------------------------------------------
class Application
{
public:
    Application() 
        : mSDLSystem()
    {
        if (!mSDLSystem.IsValid())
        {
            std::cerr << "Application startup failed: SDL could not be initialized." << std::endl;
            return;
        }

        std::cout << "Application created successfully." << std::endl;
    }

    bool IsValid() const { return mSDLSystem.IsValid(); }

    void Run()
    {
        if (!mSDLSystem.IsValid())
        {
            std::cerr << "Cannot run application: SDL initialization failed." << std::endl;
            return;
        }

        while (true)
        {
            SDL_Event event;
            if (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    break;
                }
				else if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						break;
					}
				}
            }

			SDL_SetRenderDrawColor(mSDLSystem.GetSDLRenderer(), 255, 0, 0, 255);
			SDL_RenderClear(mSDLSystem.GetSDLRenderer());
			SDL_RenderPresent(mSDLSystem.GetSDLRenderer());
        }
    }

private:
    SDLSystem mSDLSystem;
};

//------------------------------------------------------------------------------
int SDL_main(int argc, char* argv[])
{
    (void)argc; // Avoid unused parameter warning
    (void)argv;

    Application app;
    if (!app.IsValid())
    {
        return -1;
    }

    app.Run();
    return 0;
}