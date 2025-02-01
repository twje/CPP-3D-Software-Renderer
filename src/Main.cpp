// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppCore.h"
#include "Core/SDLWrappers/SDLTexture.h"

// Third party
#include <SDL.h>

//------------------------------------------------------------------------------
class ColorBuffer
{
public:
    ColorBuffer(AppContext& context)
        : mContext(context)
        , mTexture(context.mRenderer, Vector2u(context.GetWindowSize()))
        , mBuffer(context.GetWindowSize().x * context.GetWindowSize().y, 0)
    { }

    bool IsValid() const { return mTexture.IsValid(); }

    void Clear(uint32_t color)
    {
        std::fill(mBuffer.begin(), mBuffer.end(), color);
    }

    void SetPixel(int32_t x, int32_t y, int32_t color)
    {
		const Vector2i windowSize = mContext.GetWindowSize();

        if (x > 0 && x < windowSize.x && y > 0 && y < windowSize.y)
        {
            mBuffer[y * windowSize.x + x] = color;
        }
    }

    void UpdateTexture()
    {
        if (mTexture.IsValid())
        {
            SDL_UpdateTexture(
                mTexture.GetTexture(), 
                nullptr, mBuffer.data(), 
                mContext.GetWindowSize().x * sizeof(uint32_t)
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
class Camera
{
public:
    Camera()
        : Camera({ 0, 0 }, { 0.0f, 0.0f }, 90.0f)
	{ }

    Camera(Vector2i position, Vector2f rotation, float fov)
		: mPosition(position)
		, mRotation(rotation)
		, mFov(fov)
	{ }

	void SetPosition(const Vector2i& position) { mPosition = position; }
	void SetRotation(const Vector2f& rotation) { mRotation = rotation; }
	void SetFov(float fov) { mFov = fov; }

	const Vector2i& GetPosition() const { return mPosition; }
	const Vector2f& GetRotation() const { return mRotation; }
	float GetFov() const { return mFov; }

private:
	Vector2i mPosition;
	Vector2f mRotation;
	float mFov;
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

        // From -1 to 1 (in 9x9x9 cube)
        mCubePoints.reserve(9 * 9 * 9);
        mProjectedPoints.resize(9 * 9 * 9);		

        auto projectPoint = [](const Vector3f& point) -> Vector2f
        {
            const float fov = 640.0f;
            float newX = (point.x * fov) / point.z;
            float newY = (point.y * fov) / point.z;
            return Vector2f{ newX, newY };
        };
        

		for (float x = -1; x <= 1.0f; x += 0.25f)
		{
            for (float y = -1; y <= 1.0f; y += 0.25f)
            {
                for (float z = -1; z <= 1.0f; z += 0.25f)
                {					
                    mCubePoints.push_back({ x, y, z });
                }
            }
		}
    }

    virtual void OnEvent(const SDL_Event& event) override
    { 
        (void)event;
    }

    virtual void OnUpdate() override
    { 
        Vector3f cameraPosition { 0.0f, 0.0f, -5.0f };
        
        mCubeRotation.x += 0.01f;
        mCubeRotation.y += 0.01f;
        mCubeRotation.z += 0.01f;

        for (size_t i = 0; i < mCubePoints.size(); i++)
        {
			Vector3f point = mCubePoints[i];

            Vector3f transformedPoint = RotateAboutX(point, mCubeRotation.x);
            transformedPoint = RotateAboutY(transformedPoint, mCubeRotation.y);
            transformedPoint = RotateAboutZ(transformedPoint, mCubeRotation.z);
        
            // Translate the points away from the camera
            transformedPoint.z -= cameraPosition.z;

            // Project the current point
            Vector2f projected_point = Project(transformedPoint);

            // Save the projected 2D vector in the array of projected points
            mProjectedPoints[i] = projected_point;
        }
    }

    virtual void OnRender() override
    {
        mPixelRenderer->Clear(0x00000000);
        
		const Vector2i windowSize = GetContext().GetWindowSize();

		for (const Vector2f& projectedPoint : mProjectedPoints)
		{
            DrawRectangle(
                static_cast<int32_t>(projectedPoint.x + (windowSize.x * 0.5f)),
                static_cast<int32_t>(projectedPoint.y + (windowSize.y * 0.5f)),
                4,
                4,
                0xFFFFFFFF
            );
		}

		mPixelRenderer->Render();
    }

private:
	Vector3f RotateAboutX(const Vector3f& point, float angle)
	{
		const float s = sin(angle);
		const float c = cos(angle);

		return {
			point.x,
			point.y * c - point.z * s,
			point.y * s + point.z * c,
		};
	}

    Vector3f RotateAboutY(const Vector3f& point, float angle)
    {
        const float s = sin(angle);
        const float c = cos(angle);

        return {
            point.x * c - point.z * s,
            point.y,
            point.x * s + point.z * c,
        };
    }

	Vector3f RotateAboutZ(const Vector3f& point, float angle)
	{
		const float s = sin(angle);
		const float c = cos(angle);

        return {
			point.x * c - point.y * s,
			point.x * s + point.y * c,
			point.z
		};
    }

	Vector2f Project(const Vector3f& point)
	{
		const float fov = 640.0f;		
        return { (point.x * fov) / point.z, (point.y * fov) / point.z };
	}

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
		const Vector2i windowSize = GetContext().GetWindowSize();

        for (int32_t x = 0; x <= windowSize.x; x += 1)
        {
            for (int32_t y = 0; y <= windowSize.y; y += 1)
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
	std::vector<Vector3f> mCubePoints;
    std::vector<Vector2f> mProjectedPoints;    
    Vector3f mCubeRotation;
};

//------------------------------------------------------------------------------
std::unique_ptr<Application> CreateApplication()
{
	AppConfig config;
	config.mWindowTitle = "My Custom SDL App";
	config.mFullscreen = true;
	config.mUseNativeResolution = true;
	config.mMonitorIndex = 1;

	return std::make_unique<RendererApplication>(config);
}