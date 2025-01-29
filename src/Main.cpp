// Includes
//------------------------------------------------------------------------------
// Third party
#include <SDL.h>

//------------------------------------------------------------------------------
struct AppConfig
{
    uint32_t mWindowWidth = 800;
    uint32_t mWindowHeight = 600;
    std::string mWindowTitle = "SDL Application";
};

//------------------------------------------------------------------------------
struct SDLWindowDeleter 
{
    void operator()(SDL_Window* window) const 
    {
        if (window) SDL_DestroyWindow(window);
    }
};

//------------------------------------------------------------------------------
struct SDLRendererDeleter 
{
    void operator()(SDL_Renderer* renderer) const 
    {
        if (renderer) SDL_DestroyRenderer(renderer);
    }
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

    SDL_Renderer* GetSDLRenderer() { return mRenderer.get(); }

    ~SDLWindow()
    {
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
        mWindow.reset(SDL_CreateWindow(
            config.mWindowTitle.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            config.mWindowWidth,
            config.mWindowHeight,
            SDL_WINDOW_SHOWN));

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
        mRenderer.reset(SDL_CreateRenderer(mWindow.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
        if (mRenderer)
        {
            std::cout << "SDL Renderer created successfully." << std::endl;
            return true;
        }

        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }

    std::unique_ptr<SDL_Window, SDLWindowDeleter> mWindow;
    std::unique_ptr<SDL_Renderer, SDLRendererDeleter> mRenderer;
    bool mInitialized;
};

//------------------------------------------------------------------------------
// RAII Deleter for SDL_Texture
struct SDLTextureDeleter 
{
    void operator()(SDL_Texture* texture) const 
    {
        if (texture) SDL_DestroyTexture(texture);
    }
};

//------------------------------------------------------------------------------
class ColorBuffer
{
public:
    ColorBuffer(SDL_Renderer* renderer, const AppConfig& config)
        : mRenderer(renderer)
        , mWidth(config.mWindowWidth)
        , mHeight(config.mWindowHeight)
        , mBuffer(mWidth* mHeight, 0)
    {        
        mTexture.reset(SDL_CreateTexture(mRenderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            mWidth,
            mHeight));

        if (!mTexture)
        {
            SDL_Log("Failed to create texture: %s", SDL_GetError());
        }
    }

    bool IsValid() const { return mTexture != nullptr; }

    void Clear(uint32_t color)
    {
        std::fill(mBuffer.begin(), mBuffer.end(), color);
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
        if (mTexture)
        {
            SDL_UpdateTexture(mTexture.get(), nullptr, mBuffer.data(), mWidth * sizeof(uint32_t));
        }
    }

    void Render()
    {
        if (mTexture)
        {
            SDL_RenderCopy(mRenderer, mTexture.get(), nullptr, nullptr);
        }
    }

private:
    SDL_Renderer* mRenderer;
    uint32_t mWidth;
    uint32_t mHeight;
    std::vector<uint32_t> mBuffer;
    std::unique_ptr<SDL_Texture, SDLTextureDeleter> mTexture;
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
    explicit Application(const AppConfig& config)
        : mConfig(config)
        , mSDLWindow(config)
        , mPixelRenderer(mSDLWindow.GetSDLRenderer(), config)
        , mRunning(IsValid())
    { }

    bool IsValid() const
    {
        return mSDLWindow.IsValid() && mPixelRenderer.IsValid();
    }

    void Run()
    {
        if (!IsValid())
        {            
            return;
        }

        SDL_Renderer* renderer = mSDLWindow.GetSDLRenderer();
        
        while (mRunning)
        {
            ProcessEvents();
            OnUpdate();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            OnRender(mPixelRenderer);
            mPixelRenderer.Render();

            SDL_RenderPresent(renderer);
        }
    }

    AppConfig GetConfig() const { return mConfig; }

protected:
    // Lifecycle Methods (Meant for Subclassing)
    virtual void OnEvent(const SDL_Event& event) { (void)event; }
    virtual void OnUpdate() { }
    virtual void OnRender(PixelRenderer& pixelRenderer) { (void)pixelRenderer; }

private:
    void ProcessEvents()
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

    AppConfig mConfig;
    SDLWindow mSDLWindow;
    PixelRenderer mPixelRenderer;
    bool mRunning;
};

//------------------------------------------------------------------------------
class RendererApplication : public Application
{
public:
	RendererApplication(const AppConfig& config)
		: Application(config)
	{ }

    virtual void OnEvent(const SDL_Event& event) override
    { 
        (void)event;
    }

    virtual void OnUpdate() override
    { 
        
    }

    virtual void OnRender(PixelRenderer& pixelRenderer) override
    {
        pixelRenderer.Clear(0xFF0000FF);
        for (uint32_t i = 0; i < GetConfig().mWindowWidth; ++i)
        {
            pixelRenderer.SetPixel(i, 300, 0xFFFF0000);
        }
    }
};

//------------------------------------------------------------------------------
int SDL_main(int argc, char* argv[])
{
    (void)argc; // Avoid unused parameter warning
    (void)argv;

    AppConfig config;
    config.mWindowWidth = 800;
    config.mWindowHeight = 600;
    config.mWindowTitle = "My Custom SDL App";

    RendererApplication app(config);
    if (!app.IsValid())
    {
        return -1;
    }

    app.Run();
    return 0;
}