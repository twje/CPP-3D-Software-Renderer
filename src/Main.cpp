// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppConfig.h"
#include "Core/AppContext.h"
#include "Core/SDLWrappers/SDLWindow.h"
#include "Core/SDLWrappers/SDLRenderer.h"
#include "Core/SDLWrappers/SDLTexture.h"
#include "Core/Application.h"

// Third party
#include <SDL.h>

//------------------------------------------------------------------------------
class ColorBuffer
{
public:
    ColorBuffer(AppContext& context)
        : mContext(context)
        , mTexture(context.mRenderer, context.GetWindowWidth(), context.GetWindowHeight())
        , mBuffer(context.GetWindowWidth() * context.GetWindowHeight(), 0)
    { }

    bool IsValid() const { return mTexture.IsValid(); }

    void Clear(uint32_t color)
    {
        std::fill(mBuffer.begin(), mBuffer.end(), color);
    }

    void SetPixel(int32_t x, int32_t y, int32_t color)
    {
        if (x < mContext.GetWindowWidth() && y < mContext.GetWindowHeight())
        {
            mBuffer[y * mContext.GetWindowWidth() + x] = color;
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
    AppContext& mContext;
    SDLTexture mTexture;
    std::vector<uint32_t> mBuffer;
};

//------------------------------------------------------------------------------
class PixelRenderer
{
public:
    PixelRenderer(AppContext& context)
        : mColorBuffer(context)
    { }

    bool IsValid() const { return mColorBuffer.IsValid(); }

    void SetPixel(int32_t x, int32_t y, int32_t color)
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
        mPixelRenderer->Clear(0x00000000);
        DrawRectangle(300, 200, 300, 150, 0xFFFF00FF);
		mPixelRenderer->Render();
    }

private:
	void DrawRectangle(int32_t x, int32_t y, int32_t width, int32_t height, int32_t color)
	{
        for (int32_t i = 0; i <= width; i += 1)
        {
            for (int32_t j = 0; j <= height; j += 1)
            {
                const int32_t currentX = x + i;
                const int32_t currentY = y + j;
                mPixelRenderer->SetPixel(currentX, currentY, color);
            }
        }
	}

    void DrawGrid()
    {
        const int32_t interval = 10;
        const int32_t width = GetContext().GetWindowWidth();
		const int32_t height = GetContext().GetWindowHeight();

        for (int32_t x = 0; x <= width; x += 1)
        {
            for (int32_t y = 0; y <= height; y += 1)
            {
				if (x % interval == 0 || y % interval == 0)
				{
					mPixelRenderer->SetPixel(x, y, 0xFFFFFFFF);
				}
            }
        }
    }

    void DrawVerticalLine(int32_t x, int32_t yStart, int32_t yEnd, int32_t color)
    {
        if (yStart > yEnd)
        {
            std::swap(yStart, yEnd);
        }

        for (int32_t y = yStart; y <= yEnd; ++y)
        {
            mPixelRenderer->SetPixel(x, y, color);
        }
    }

    void DrawHorizontalLine(int32_t y, int32_t xStart, int32_t xEnd, int32_t color)
    {
        if (xStart > xEnd)
        {
            std::swap(xStart, xEnd);
        }

        for (int32_t x = xStart; x <= xEnd; ++x)
        {
            mPixelRenderer->SetPixel(x, y, color);
        }
    }

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
    config.mUseNativeResolution = true;
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