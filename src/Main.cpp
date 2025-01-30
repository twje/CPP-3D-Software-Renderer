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
    bool mFullscreen = false;
	bool mUseNativeResolution = true;
	int32_t mMonitorIndex = 0;
};

//------------------------------------------------------------------------------
class SDLWindow
{
public:
    explicit SDLWindow(const AppConfig& config)
        : mWindow(nullptr)
        , mWindowWidth(config.mWindowWidth)
        , mWindowHeight(config.mWindowHeight)
    {
        CreateWindow(config);        
    }

    ~SDLWindow()
    {
        if (mWindow)
        {
            SDL_DestroyWindow(mWindow);
        }
    }

	SDL_Window* GetSDLWindow() { return mWindow; }
    bool IsValid() const { return mWindow != nullptr; }    
    uint32_t GetWindowWidth() const { return mWindowWidth; }
    uint32_t GetWindowHeight() const { return mWindowHeight; }

    // Delete copy and assignment to prevent accidental re-initialization
    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

private:
    void CreateWindow(const AppConfig& config)
    {
        uint32_t windowFlags = SDL_WINDOW_SHOWN;
        int32_t displayIndex = config.mMonitorIndex;

        if (config.mFullscreen)
        {
            windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        SDL_DisplayMode displayMode;
        if (SDL_GetCurrentDisplayMode(displayIndex, &displayMode) != 0)
        {
            SDL_Log("Failed to get display mode: %s", SDL_GetError());
            return;
        }

        if (config.mUseNativeResolution)
        {
            mWindowWidth = displayMode.w;
            mWindowHeight = displayMode.h;
        }

        int32_t posX = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex);
        int32_t posY = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex);

        mWindow = SDL_CreateWindow(
            config.mWindowTitle.c_str(),
            posX, posY,
            mWindowWidth, mWindowHeight,
            windowFlags);

        if (mWindow)
        {
            std::cout << "SDL Window created successfully on monitor " << displayIndex << "." << std::endl;
            std::cout << "Window Size: " << mWindowWidth << "x" << mWindowHeight << std::endl;
            return;
        }

        SDL_Log("Failed to create window: %s", SDL_GetError());
        return;
    }

    SDL_Window* mWindow;    
    uint32_t mWindowWidth;
    uint32_t mWindowHeight;
};

//------------------------------------------------------------------------------
class SDLRenderer
{
public:
    explicit SDLRenderer(SDLWindow& window)
        : mRenderer(nullptr)
    {		
        if (SDL_Window* sdlWindow = window.GetSDLWindow())
        {
            mRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (!mRenderer)
            {
                SDL_Log("Failed to create renderer: %s", SDL_GetError());
            }
            else
            {
                std::cout << "SDL Renderer created successfully." << std::endl;
            }
        }
    }

    ~SDLRenderer()
    {
        if (mRenderer)
        {
            SDL_DestroyRenderer(mRenderer);
        }
    }

    bool IsValid() const { return mRenderer != nullptr; }

    SDL_Renderer* GetSDLRenderer() { return mRenderer; }

    // Delete copy and assignment to prevent accidental re-initialization
    SDLRenderer(const SDLRenderer&) = delete;
    SDLRenderer& operator=(const SDLRenderer&) = delete;

private:
    SDL_Renderer* mRenderer;
};

//------------------------------------------------------------------------------
struct SDLContext
{
    SDLWindow mWindow;
    SDLRenderer mRenderer;

    explicit SDLContext(const AppConfig& config)
        : mWindow(config)
        , mRenderer(mWindow)
    { }

    bool IsValid() const { return mWindow.IsValid() && mRenderer.IsValid(); }
};

//------------------------------------------------------------------------------
class SDLTexture
{
public:
    explicit SDLTexture(SDLRenderer& renderer, uint32_t width, uint32_t height)
		: mTexture(nullptr)
    {
        mTexture = SDL_CreateTexture(renderer.GetSDLRenderer(),
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height);

        if (!mTexture)
        {
            SDL_Log("Failed to create texture: %s", SDL_GetError());
        }
    }

    ~SDLTexture()
    {
        if (mTexture)
        {
            SDL_DestroyTexture(mTexture);
        }
    }

    bool IsValid() const { return mTexture != nullptr; }
    SDL_Texture* GetTexture() { return mTexture; }

private:
    SDL_Texture* mTexture;
};

//------------------------------------------------------------------------------
class ColorBuffer
{
public:
    ColorBuffer(SDLContext& context)
        : mContext(context)
        , mTexture(context.mRenderer, context.mWindow.GetWindowWidth(), context.mWindow.GetWindowHeight())
        , mBuffer(context.mWindow.GetWindowWidth() * context.mWindow.GetWindowHeight(), 0)
    { }

    bool IsValid() const { return mTexture.IsValid(); }

    void Clear(uint32_t color)
    {
        std::fill(mBuffer.begin(), mBuffer.end(), color);
    }

    void SetPixel(uint32_t x, uint32_t y, uint32_t color)
    {
        if (x < mContext.mWindow.GetWindowWidth() && y < mContext.mWindow.GetWindowHeight())
        {
            mBuffer[y * mContext.mWindow.GetWindowWidth() + x] = color;
        }
    }

    void UpdateTexture()
    {
        if (mTexture.IsValid())
        {
            SDL_UpdateTexture(
                mTexture.GetTexture(), 
                nullptr, mBuffer.data(), 
                mContext.mWindow.GetWindowWidth() * sizeof(uint32_t)
            );
        }
    }

    void Render()
    {
        if (mTexture.IsValid())
        {
            SDL_RenderCopy(mContext.mRenderer.GetSDLRenderer(), mTexture.GetTexture(), nullptr, nullptr);
        }
    }

private:
    SDLContext& mContext;
    SDLTexture mTexture;
    std::vector<uint32_t> mBuffer;
};

//------------------------------------------------------------------------------
class PixelRenderer
{
public:
    PixelRenderer(SDLContext& context)
        : mColorBuffer(context)
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
        : mContext(config)
        , mRunning(IsValid())
    { }

    bool IsValid() const { return mContext.IsValid(); }

    void Run()
    {
        if (!IsValid())
        {            
            return;
        }

        SDL_Renderer* renderer = mContext.mRenderer.GetSDLRenderer();
        
        OnCreate();

        while (mRunning)
        {
            ProcessEvents();
            OnUpdate();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            OnRender();
            SDL_RenderPresent(renderer);
        }
    }

    const SDLContext& GetContext() const { return mContext; }
    SDLContext& GetContext() { return mContext; }

protected:
    // Lifecycle Methods (Meant for Subclassing)
    virtual void OnCreate() { }
    virtual void OnEvent(const SDL_Event& event) { (void)event; }
    virtual void OnUpdate() { }
    virtual void OnRender() { }

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
    
	SDLContext mContext;
    bool mRunning;
};

//------------------------------------------------------------------------------
class RendererApplication : public Application
{
public:
	RendererApplication(const AppConfig& config)
		: Application(config)
	{ }

    virtual void OnCreate() override
    {
		mPixelRenderer = std::make_unique<PixelRenderer>(GetContext());
    }

    virtual void OnEvent(const SDL_Event& event) override
    { 
        (void)event;
    }

    virtual void OnUpdate() override
    { 
        
    }

    virtual void OnRender() override
    {
		const SDLWindow& window = GetContext().mWindow;

        mPixelRenderer->Clear(0xFF0000FF);
        for (uint32_t i = 0; i < window.GetWindowWidth(); ++i)
        {
            mPixelRenderer->SetPixel(i, 300, 0xFFFF0000);
        }
		mPixelRenderer->Render();
    }

private:
    std::unique_ptr<PixelRenderer> mPixelRenderer;
};

//------------------------------------------------------------------------------
bool InitializeSDL()
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
int SDL_main(int argc, char* argv[])
{
    (void)argc; // Avoid unused parameter warning
    (void)argv;

    if (!InitializeSDL())
    {
        return -1;
    }

    AppConfig config;
    config.mWindowTitle = "My Custom SDL App";
    config.mFullscreen = true;
    config.mUseNativeResolution = false;
    config.mMonitorIndex = 1;

    RendererApplication app(config);
    if (!app.IsValid())
    {
        SDL_Quit();
        return -1;
    }

    app.Run();
    SDL_Quit();
    return 0;
}