// Includes
//------------------------------------------------------------------------------
// Application
#include "Mesh.h"

// Core
#include "Core/AppCore.h"
#include "Core/Utils.h"
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
		
        mMesh = CreateMeshFromOBJFile(ResolveAssetPath("f22.obj"));
		mTrianglesToRender.resize(mMesh->FaceCount());
    }

    virtual void OnEvent(const SDL_Event& event) override
    { 
        (void)event;
    }

    virtual void OnUpdate() override
    { 
        static std::vector<Vector3f> faceVertices(3);
        
        const Vector2i windowSize = GetContext().GetWindowSize();
        const Vector3f cameraPosition { 0.0f, 0.0f, -5.0f };
                
		mMesh->AddRotation({ 0.01f, 0.01f, 0.01f });
        
		// Build up a list of projected triangles to render
        for (size_t i = 0; i < mMesh->FaceCount(); i++)
        {
			const Face& face = mMesh->GetFace(i);

            faceVertices[0] = mMesh->GetVertex(face.a);
            faceVertices[1] = mMesh->GetVertex(face.b);
            faceVertices[2] = mMesh->GetVertex(face.c);
            
            Triangle projectedTriangle;

			for (size_t j = 0; j < 3; j++)
			{
				Vector3f transformedVertex = faceVertices[j];

				const Vector3f& rotation = mMesh->GetRotation();
                transformedVertex = RotateAboutX(transformedVertex, rotation.x);
                transformedVertex = RotateAboutY(transformedVertex, rotation.y);
                transformedVertex = RotateAboutZ(transformedVertex, rotation.z);

                // Translate the points away from the camera
                transformedVertex.z -= cameraPosition.z;

                // Project the current point
                Vector2f projectedPoint = Project(transformedVertex);

                // Scale and translate the projected points to the middle of the screen
                projectedPoint.x += windowSize.x * 0.5f;
                projectedPoint.y += windowSize.y * 0.5f;

                projectedTriangle.SetPoint(j, projectedPoint);
			}

            mTrianglesToRender[i] = projectedTriangle;
        }
    }

    virtual void OnRender() override
    {
        mPixelRenderer->Clear(0x00000000);
        
        for (size_t i = 0; i < mMesh->FaceCount(); i++)
        {
			const Triangle& triangle = mTrianglesToRender[i];            
			DrawTriangle(triangle);
        }

		mPixelRenderer->Render();
    }

private:
    void DrawTriangle(const Triangle& triangle)
    {
        DrawLine(
            static_cast<int32_t>(triangle.GetPoint(0).x),
            static_cast<int32_t>(triangle.GetPoint(0).y),
            static_cast<int32_t>(triangle.GetPoint(1).x),
            static_cast<int32_t>(triangle.GetPoint(1).y)
        );

        DrawLine(
            static_cast<int32_t>(triangle.GetPoint(1).x),
            static_cast<int32_t>(triangle.GetPoint(1).y),
            static_cast<int32_t>(triangle.GetPoint(2).x),
            static_cast<int32_t>(triangle.GetPoint(2).y)
        );

        DrawLine(
            static_cast<int32_t>(triangle.GetPoint(2).x),
            static_cast<int32_t>(triangle.GetPoint(2).y),
            static_cast<int32_t>(triangle.GetPoint(0).x),
            static_cast<int32_t>(triangle.GetPoint(0).y)
        );
    }

    void DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
    {
		// Use DDA algorithm to draw a line
        const int32_t deltaX = x1 - x0;
        const int32_t deltaY = y1 - y0;

		// Take the greater of the two deltas
        const int32_t sideLength = abs(deltaX) >= abs(deltaY) ? abs(deltaX) : abs(deltaY);

        const float xInc = deltaX / static_cast<float>(sideLength);
        const float yInc = deltaY / static_cast<float>(sideLength);

		float currentX = static_cast<float>(x0);
        float currentY = static_cast<float>(y0);

        for (int32_t i = 0; i <= sideLength; i++)
        {
            int32_t pixelX = static_cast<int32_t>(round(currentX));
            int32_t pixelY = static_cast<int32_t>(round(currentY));

            mPixelRenderer->SetPixel(pixelX, pixelY, 0xFFFFFFFF);

			currentX += xInc;
			currentY += yInc;
        }
    }

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
	std::vector<Triangle> mTrianglesToRender;
	std::unique_ptr<Mesh> mMesh;
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