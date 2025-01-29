// Includes
//------------------------------------------------------------------------------
// Third party
#include <SDL.h>

//------------------------------------------------------------------------------
struct AppConfig
{
    uint32_t mWindowWidth = 800;
    uint32_t mWindowHeight = 600;
    std::string windowTitle = "SDL Application";
};

//------------------------------------------------------------------------------
class SDLWindow
{
public:
    explicit SDLWindow(const AppConfig& config, uint32_t flags = SDL_INIT_VIDEO)
        : mWindow(nullptr)
        , mRenderer(nullptr)
        , mInitialized(false)
    {
        if (!InitializeSDL(flags) || !CreateWindow(config) || !CreateRenderer())
        {
            return;
        }
    }

    bool IsValid() const { return mInitialized && mWindow && mRenderer; }

    SDL_Renderer* GetSDLRenderer() { return mRenderer; }

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

    bool CreateWindow(const AppConfig& config)
    {
        mWindow = SDL_CreateWindow(
            config.windowTitle.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config.mWindowWidth,
            config.mWindowHeight,
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
    ColorBuffer(SDL_Renderer* renderer, const AppConfig& config)
        : mRenderer(renderer)
        , mWidth(config.mWindowWidth)
        , mHeight(config.mWindowHeight)
        , mInitialized(false)
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
            return;
        }

        mInitialized = true;
    }

    ~ColorBuffer()
    {
        delete[] mBuffer;
        if (mTexture)
        {
            SDL_DestroyTexture(mTexture);
        }
    }

    bool IsValid() const { return mInitialized; }

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

private:
    SDL_Renderer* mRenderer;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t* mBuffer;
    SDL_Texture* mTexture;
    bool mInitialized;
};

//------------------------------------------------------------------------------
class PixelRenderer
{
public:
    PixelRenderer(SDL_Renderer* renderer, const AppConfig& config)
        : mColorBuffer(renderer, config)
    { }

    bool IsValid() const { return mColorBuffer.IsValid(); }

    void SetPixel(uint32_t x, uint32_t y, uint32_t color)
    {
		mColorBuffer.SetPixel(x, y, color);
    }

    void Clear(uint32_t color)
    {
		mColorBuffer.Clear(color);
    }

    void Render()
    {
        mColorBuffer.UpdateTexture();
		mColorBuffer.Render();
    }

private:
	ColorBuffer mColorBuffer;
};

//------------------------------------------------------------------------------
class Application
{
public:
    Application(const AppConfig& config)
        : mConfig(config)
        , mSDLWindow(config)
        , mPixelRenderer(mSDLWindow.GetSDLRenderer(), config)
    {
        if (!mSDLWindow.IsValid())
        {
            std::cerr << "Application startup failed: SDL could not be initialized." << std::endl;
            return;
        }

        std::cout << "Application created successfully." << std::endl;
    }

    bool IsValid() const 
    { 
        return mSDLWindow.IsValid() && mPixelRenderer.IsValid();
    }

    void Run()
    {
        if (!IsValid())
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
            mPixelRenderer.Clear(0xFF0000FF);
			for (uint32_t i = 0; i < mConfig.mWindowWidth; ++i)
			{
                mPixelRenderer.SetPixel(i, 300, 0xFFFF0000);
			}            

			SDL_SetRenderDrawColor(mSDLWindow.GetSDLRenderer(), 0, 0, 0, 255);
			SDL_RenderClear(mSDLWindow.GetSDLRenderer());
                        
            mPixelRenderer.Render();
			SDL_RenderPresent(mSDLWindow.GetSDLRenderer());
        }
    }

private:
    AppConfig mConfig;
    SDLWindow mSDLWindow;
    PixelRenderer mPixelRenderer;
};

//------------------------------------------------------------------------------
int SDL_main(int argc, char* argv[])
{
    (void)argc; // Avoid unused parameter warning
    (void)argv;

    AppConfig config;
    config.mWindowWidth = 800;
    config.mWindowHeight = 600;
    config.windowTitle = "My Custom SDL App";

    Application app(config);
    if (!app.IsValid())
    {
        return -1;
    }

    app.Run();
    return 0;
}