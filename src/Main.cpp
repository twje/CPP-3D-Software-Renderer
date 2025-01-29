// Includes
//------------------------------------------------------------------------------
// Third party
#include <SDL.h>

//------------------------------------------------------------------------------
class SDLWindow
{
public:
    explicit SDLWindow(uint32_t flags = SDL_INIT_VIDEO)
        : mWindow(nullptr)
		, mRenderer(nullptr)
        , mInitialized(false)
    {
        if (!InitializeSDL(flags) || !CreateWindow() || !CreateRenderer())
        {
            return;  // Failure is logged inside helper functions
        }
    }

    ~SDLWindow()
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
    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

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
class ColorBuffer
{
public:
    ColorBuffer(SDL_Renderer* renderer, uint32_t width, uint32_t height)
        : mRenderer(renderer)
        , mWidth(width)
        , mHeight(height)
    {
        mBuffer = new uint32_t[mWidth * mHeight];

        // Create an SDL texture for rendering
        mTexture = SDL_CreateTexture(mRenderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            mWidth,
            mHeight);

        if (!mTexture)
        {
            SDL_Log("Failed to create texture: %s", SDL_GetError());
        }
    }

    ~ColorBuffer()
    {
        delete[] mBuffer;
        if (mTexture)
        {
            SDL_DestroyTexture(mTexture);
        }
    }

    void Clear(uint32_t color)
    {
        std::fill(mBuffer, mBuffer + (mWidth * mHeight), color);
    }

    void SetPixel(uint32_t x, uint32_t y, uint32_t color)
    {
        if (x < mWidth && y < mHeight)
        {
            mBuffer[y * mWidth + x] = color;
        }
    }

    void UpdateTexture()
    {
        SDL_UpdateTexture(mTexture, nullptr, mBuffer, mWidth * sizeof(uint32_t));
    }

    void Render()
    {
        SDL_RenderCopy(mRenderer, mTexture, nullptr, nullptr);
    }

    uint32_t GetWidth() const { return mWidth; }
    uint32_t GetHeight() const { return mHeight; }

private:
    SDL_Renderer* mRenderer;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t* mBuffer;
    SDL_Texture* mTexture;  
};

//------------------------------------------------------------------------------
class Application
{
public:
    Application() 
        : mSDLWindow()
		, mColorBuffer(mSDLWindow.GetSDLRenderer(), 800, 600)
    {
        if (!mSDLWindow.IsValid())
        {
            std::cerr << "Application startup failed: SDL could not be initialized." << std::endl;
            return;
        }

        std::cout << "Application created successfully." << std::endl;
    }

    bool IsValid() const { return mSDLWindow.IsValid(); }

    void Run()
    {
        if (!mSDLWindow.IsValid())
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

            // Update buffer
            mColorBuffer.Clear(0xFF0000FF);
			for (uint32_t i = 0; i < 800; ++i)
			{
				mColorBuffer.SetPixel(i, 300, 0xFFFF0000);
			}            
            mColorBuffer.UpdateTexture();

			SDL_SetRenderDrawColor(mSDLWindow.GetSDLRenderer(), 0, 0, 0, 255);
			SDL_RenderClear(mSDLWindow.GetSDLRenderer());
            mColorBuffer.Render();
			SDL_RenderPresent(mSDLWindow.GetSDLRenderer());
        }
    }

private:
    SDLWindow mSDLWindow;
    ColorBuffer mColorBuffer;
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