// Includes
//------------------------------------------------------------------------------
// Application
#include "Mesh.h"
#include "Matrix.h"
#include "Light.h"
#include "Texture.h"

// Core
#include "Core/AppCore.h"
#include "Core/Utils.h"
#include "Core/SDLWrappers/SDLTexture.h"

// Third party
#include <SDL.h>
#include <glm/glm.hpp>

// System
#include <functional>

/*
    TODO:
	Pressing 1 displays the wireframe and a small red dot for each triangle vertex
    Pressing 2 displays only the wireframe lines
    Pressing 3 displays filled triangles with a solid color
    Pressing 4 displays both filled triangles and wireframe lines
    Pressing c we should enable back-face culling
    Pressing d we should disable the back-face culling
*/

// Type Alias
//------------------------------------------------------------------------------
using RasterTriangleFunc = std::function<void(const glm::ivec2& point, const std::array<Vertex, 3>& vertices)>;

//------------------------------------------------------------------------------
enum class CullMethod
{
	None,
	Backface
};

//------------------------------------------------------------------------------
enum class RenderMethod
{
	Wireframe,
    WireframeWithVertices,
	WireframeTriangle,
    WireframeTriangleWire
};

//------------------------------------------------------------------------------
class ColorBuffer
{
public:
    ColorBuffer(AppContext& context)
        : mContext(context)
        , mTexture(context.mRenderer, glm::uvec2(context.GetWindowSize()))
        , mBuffer(context.GetWindowSize().x * context.GetWindowSize().y, 0)
    { }

    bool IsValid() const { return mTexture.IsValid(); }

    void Clear(uint32_t color)
    {
        std::fill(mBuffer.begin(), mBuffer.end(), color);
    }

    void SetPixel(int32_t x, int32_t y, int32_t color)
    {
		const glm::ivec2 windowSize = mContext.GetWindowSize();

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
class RendererApplication : public Application
{
public:
	RendererApplication(const AppConfig& config)
		: Application(config)
		, mDirectionalLight({ 0.0f, 0.0f, 1.0f })
	{ }

    virtual void OnCreate() override
    {
		mPixelRenderer = std::make_unique<PixelRenderer>(GetContext());

        mMesh = CreateMeshFromOBJFile(ResolveAssetPath("f22.obj"));
		mTrianglesToRender.reserve(mMesh->FaceCount());
		mTexture = std::make_unique<Texture>(ResolveAssetPath("wall_cobblestone.png"));
    }

    virtual void OnEvent(const SDL_Event& event) override
    { 
		(void)event;
    }

    virtual void OnUpdate() override
    { 
        static std::vector<glm::vec3> faceVertices(3);        
        
        const  glm::vec2 windowSize = glm::vec2(GetContext().GetWindowSize());
        const glm::vec3 cameraPosition { 0.0f, 0.0f, 0.0f };
               		
		mTrianglesToRender.clear();
        std::array<glm::vec4, 3> transformedVertices;

		mMesh->SetScale({ 1.0f, 1.0f, 1.0f });		
		mMesh->AddRotation({ 1.0f, 0.0f, 0.0f });
		mMesh->SetTranslation({ 0.0f, 0.0f, 10.0f });

        glm::mat4 modelMatrix = ComputeModelMatrix(*mMesh);
        
        glm::mat4 projectionMatrix = CreatePerspectiveProjectionMatrix(
            60.0f,
            windowSize.y / windowSize.x,
            0.2f,
            110.0f
        );

		// Build up a list of projected triangles to render
        for (size_t i = 0; i < mMesh->FaceCount(); i++)
        {
			const Face& face = mMesh->GetFace(i);

			// Transform the vertices
            for (size_t j = 0; j < 3; j++)
            {
                glm::vec3 vertex = mMesh->GetVertex(face.mVertexIndicies[j]);
                transformedVertices[j] = modelMatrix * glm::vec4(vertex, 1.0f);                
				assert(transformedVertices[j].w == 1.0f);
            }

			glm::vec3 faceNormal = ComputeFaceNormal(transformedVertices);

			// Author converted 'transformedVertices' to vec3 (ignore this for now)
            if (IsTriangleFrontFaceVisibleToCamera(cameraPosition, faceNormal, transformedVertices))  // Backface culling
            {
                Triangle projectedTriangle;

                // Project the transformed vertices
                for (size_t j = 0; j < 3; j++)
                {
					Vertex vertex;

                    vertex.mPoint = ProjectVec4(projectionMatrix, transformedVertices[j]);
                    
                    // Scale into the view
                    vertex.mPoint.x *= windowSize.x * 0.5f;
                    vertex.mPoint.y *= windowSize.y * 0.5f;

                    // Translate the projected points to the middle of the screen
                    vertex.mPoint.x += windowSize.x * 0.5f;
                    vertex.mPoint.y += windowSize.y * 0.5f;

                    projectedTriangle.SetVertex(j, vertex);
                }
                
			    // Calculate the average depth of the triangle
			    const float averageDepth = (transformedVertices[0].z + transformedVertices[1].z + transformedVertices[2].z) / 3.0f;
			    projectedTriangle.SetAverageDepth(averageDepth);
                float lightIntensity = mDirectionalLight.CalculateLightIntensity(faceNormal);

                uint32_t shadedColor = ApplyLightIntensity(projectedTriangle.GetColor(), lightIntensity);
                projectedTriangle.SetColor(shadedColor);

                mTrianglesToRender.push_back(projectedTriangle);
            }
        }

        // Painters algorithm
        // Sort faces by average depth. This is a temporary solution until a depth buffer is added.
        std::sort(mTrianglesToRender.begin(), mTrianglesToRender.end(), [](const Triangle& a, const Triangle& b)
        {
            return a.GetAverageDepth() > b.GetAverageDepth();
        });
    }

    virtual void OnRender() override
    {
        mPixelRenderer->Clear(0x00000000);
        
        for (Triangle& triangle : mTrianglesToRender)
        {
            DrawTexturedTriangle(triangle, *mTexture);			
        }

		mPixelRenderer->Render();
    }

private:
    glm::mat4 ComputeModelMatrix(const Mesh& mesh)
    {
		glm::mat4 model = glm::mat4(1.0f);
        (void)mesh;

		model *= CreateTranslationMatrix(mesh.GetTranslation());    // Applied 5th
	    model *= CreateRotateAboutZMatrix(mesh.GetRotation().z);    // Applied 4th
		model *= CreateRotateAboutYMatrix(mesh.GetRotation().y);	// Applied 3rd
        model *= CreateRotateAboutXMatrix(mesh.GetRotation().x);    // Applied 2nd  
		model *= CreateScaleMatrix(mesh.GetScale());                // Applied 1st

		return model;
    }

    glm::vec3 ComputeFaceNormal(const std::array<glm::vec4, 3>& transformedVertices)
    {
        // Check backface culling
        glm::vec3 vectorA = transformedVertices[0];  /*   A   */
        glm::vec3 vectorB = transformedVertices[1];  /*  / \  */
        glm::vec3 vectorC = transformedVertices[2];  /* C---B */

        // Get the vector subtraction of B-A and C-A
        glm::vec3 vectorAB = vectorB - vectorA;
        glm::vec3 vectorAC = vectorC - vectorA;
        vectorAB = glm::normalize(vectorAB);
        vectorAC = glm::normalize(vectorAC);

        // Compute the face normal (using cross product to find perpendicular)
        glm::vec3 faceNormal = glm::cross(vectorAB, vectorAC);
        faceNormal = glm::normalize(faceNormal);

		return faceNormal;
    }

    bool IsTriangleFrontFaceVisibleToCamera(const glm::vec3& cameraPosition, const glm::vec3& faceNormal, const std::array<glm::vec4, 3>& transformedVertices)
    {
        glm::vec3 triangleMidpoint = (transformedVertices[0] + transformedVertices[1] + transformedVertices[2]) / 3.0f;

        // Find the vector between vertex A in the triangle and the camera origin
        glm::vec3 cameraRay = cameraPosition - triangleMidpoint;
		cameraRay = glm::normalize(cameraRay);        

		float dotNormalCamera = glm::dot(faceNormal, cameraRay);

		return dotNormalCamera > 0.0f;
    }
      
	void DrawTriangleWireframe(const Triangle& triangle, uint32_t color)
	{
		DrawLine(triangle.GetVertex(0).mPoint, triangle.GetVertex(1).mPoint, color);
		DrawLine(triangle.GetVertex(1).mPoint, triangle.GetVertex(2).mPoint, color);
		DrawLine(triangle.GetVertex(2).mPoint, triangle.GetVertex(0).mPoint, color);
	}

    void DrawTexturedTriangle(const Triangle& triangle, const Texture& texture)
    {
		(void)texture;
		RasterTriangle(triangle, [&](const glm::ivec2& point, const std::array<Vertex, 3>& vertices)
		{
            (void)vertices;
			mPixelRenderer->SetPixel(point.x, point.y, 0xffffffff);
		});
    }

	void DrawFilledTriangle(const Triangle& triangle)
	{
		RasterTriangle(triangle, [&](const glm::ivec2& point, const std::array<Vertex, 3>& vertices)
		{
			(void)vertices;
			mPixelRenderer->SetPixel(point.x, point.y, triangle.GetColor());
		});
	}

    void RasterTriangle(const Triangle& triangle, const RasterTriangleFunc& callback)
    {
        // Extract triangle vertices
        std::array<Vertex, 3> vertices { triangle.GetVertex(0), triangle.GetVertex(1), triangle.GetVertex(2) };

        // Sort vertices by y-coordinate (ascending)
        std::sort(std::begin(vertices), std::end(vertices), [](const Vertex& a, const Vertex& b) {
            return a.mPoint.y < b.mPoint.y;
        });

        // Store integer points
        const glm::ivec2& point0 = vertices[0].mPoint;
        const glm::ivec2& point1 = vertices[1].mPoint;
        const glm::ivec2& point2 = vertices[2].mPoint;

        /*
                 (x0,y0)
                   / \
                  /   \
                 /     \
                /       \
               /         \
           (x1,y1)------(x2,y2)
        */
        const float longSlope = (point2.y - point0.y == 0)
            ? 0.0f
            : static_cast<float>(point2.x - point0.x) / (point2.y - point0.y);
        
        {
            const float shortSlope = (point1.y - point0.y == 0)
                ? 0.0f
                : static_cast<float>(point1.x - point0.x) / (point1.y - point0.y);

            if (point0.y - point1.y != 0)
            {
                for (int32_t y = point0.y; y <= point1.y; y++)
                {
                    int32_t xStart = static_cast<int32_t>(point1.x + (y - point1.y) * shortSlope);
                    int32_t xEnd = static_cast<int32_t>(point0.x + (y - point0.y) * longSlope);

                    if (xStart > xEnd)
                    {
                        std::swap(xStart, xEnd);
                    }

                    for (int32_t x = xStart; x < xEnd; x++)
                    {
                        callback({ x, y }, vertices);
                    }
                }
            }
        }

        /*
           (x0,y0)------(x1,y1)
               \         /
                \       /
                 \     /
                  \   /
                   \ /
                 (x2,y2)
        */
        {
            const float shortSlope = (point2.y - point1.y == 0)
                ? 0.0f
                : static_cast<float>(point2.x - point1.x) / (point2.y - point1.y);

            if (point2.y - point1.y != 0)
            {
                for (int32_t y = point1.y; y <= point2.y; y++)
                {
                    int32_t xStart = static_cast<int32_t>(point1.x + (y - point1.y) * shortSlope);
                    int32_t xEnd = static_cast<int32_t>(point0.x + (y - point0.y) * longSlope);

                    if (xStart > xEnd)
                    {
                        std::swap(xStart, xEnd);
                    }

                    for (int32_t x = xStart; x < xEnd; x++)
                    {
                        callback({ x, y }, vertices);
                    }
                }
            }
        }
    }    

    void DrawLine(const glm::ivec2& point0, const glm::ivec2& point1, uint32_t color)
    {
		// Use DDA algorithm to draw a line
        const int32_t deltaX = point1.x - point0.x;
        const int32_t deltaY = point1.y - point0.y;

		// Take the greater of the two deltas
        const int32_t sideLength = abs(deltaX) >= abs(deltaY) ? abs(deltaX) : abs(deltaY);

        const float xInc = deltaX / static_cast<float>(sideLength);
        const float yInc = deltaY / static_cast<float>(sideLength);

		float currentX = static_cast<float>(point0.x);
        float currentY = static_cast<float>(point0.y);

        for (int32_t i = 0; i <= sideLength; i++)
        {
            int32_t pixelX = static_cast<int32_t>(round(currentX));
            int32_t pixelY = static_cast<int32_t>(round(currentY));

            mPixelRenderer->SetPixel(pixelX, pixelY, color);

			currentX += xInc;
			currentY += yInc;
        }
    }

    glm::vec3 RotateAboutX(const glm::vec3& point, float angle)
	{
		const float s = sin(angle);
		const float c = cos(angle);

		return {
			point.x,
			point.y * c - point.z * s,
			point.y * s + point.z * c,
		};
	}

    glm::vec3 RotateAboutY(const glm::vec3& point, float angle)
    {
        const float s = sin(angle);
        const float c = cos(angle);

        return {
            point.x * c - point.z * s,
            point.y,
            point.x * s + point.z * c,
        };
    }

    glm::vec3 RotateAboutZ(const glm::vec3& point, float angle)
	{
		const float s = sin(angle);
		const float c = cos(angle);

        return {
			point.x * c - point.y * s,
			point.x * s + point.y * c,
			point.z
		};
    }

    glm::vec2 Project(const glm::vec3& point)
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
		const glm::ivec2 windowSize = GetContext().GetWindowSize();

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
    std::unique_ptr<Texture> mTexture;
    DirectionalLight mDirectionalLight;
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