// Includes
//------------------------------------------------------------------------------
// Application
#include "Mesh.h"
#include "Matrix.h"
#include "Light.h"

// Core
#include "Core/AppCore.h"
#include "Core/Utils.h"
#include "Core/SDLWrappers/SDLTexture.h"

// Third party
#include <SDL.h>

#include <glm/glm.hpp>

/*
    TODO:
	Pressing 1 displays the wireframe and a small red dot for each triangle vertex
    Pressing 2 displays only the wireframe lines
    Pressing 3 displays filled triangles with a solid color
    Pressing 4 displays both filled triangles and wireframe lines
    Pressing c we should enable back-face culling
    Pressing d we should disable the back-face culling
*/

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
		, mDirectionalLight({ 0.0f, -1.0f, 1.0f })
	{ }

    virtual void OnCreate() override
    {
		mPixelRenderer = std::make_unique<PixelRenderer>(GetContext());
		
        mMesh = CreateMeshFromOBJFile(ResolveAssetPath("f22.obj"));
		mTrianglesToRender.reserve(mMesh->FaceCount());
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
		//mMesh->AddRotation({ 1.0f, 1.0f, 1.0f });
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

            faceVertices[0] = mMesh->GetVertex(face.a);
            faceVertices[1] = mMesh->GetVertex(face.b);
            faceVertices[2] = mMesh->GetVertex(face.c);



			// Transform the vertices
            for (size_t j = 0; j < 3; j++)
            {
                glm::vec4 transformedVertex = modelMatrix * glm::vec4(faceVertices[j], 1.0f);
                transformedVertices[j] = transformedVertex;
				assert(transformedVertex.w == 1.0f);
            }

			// Author converted 'transformedVertices' to vec3 (ignore this for now)
            if (IsTriangleFrontFaceVisibleToCamera(cameraPosition, transformedVertices))  // Backface culling
            {
                Triangle projectedTriangle;

                // Project the transformed vertices
                for (size_t j = 0; j < 3; j++)
                {
                    glm::vec2 projectedPoint = ProjectVec4(projectionMatrix, transformedVertices[j]);
                    
                    // Scale into the view
                    projectedPoint.x *= windowSize.x * 0.5f;
                    projectedPoint.y *= windowSize.y * 0.5f;

                    // Translate the projected points to the middle of the screen
                    projectedPoint.x += windowSize.x * 0.5f;
                    projectedPoint.y += windowSize.y * 0.5f;					

                    projectedTriangle.SetPoint(j, projectedPoint);
                }
                
			    // Calculate the average depth of the triangle
			    const float averageDepth = (transformedVertices[0].z + transformedVertices[1].z + transformedVertices[2].z) / 3.0f;
			    projectedTriangle.SetAverageDepth(averageDepth);

				// Apply directional light flat shading
                const float lightIntensity = mDirectionalLight.CalculateLightIntensity(transformedVertices);
                uint32_t shadedColor = ApplyLightIntensity(projectedTriangle.GetColor(), lightIntensity);
				projectedTriangle.SetColor(shadedColor);

                mTrianglesToRender.push_back(projectedTriangle);
            }
        }
    }

    virtual void OnRender() override
    {
        mPixelRenderer->Clear(0x00000000);
        
		// Painters algorithm
		// Sort faces by average depth. This is a temporary solution until a depth buffer is added.
		std::sort(mTrianglesToRender.begin(), mTrianglesToRender.end(), [](const Triangle& a, const Triangle& b)
		{
			return a.GetAverageDepth() > b.GetAverageDepth();
		});

        for (Triangle& triangle : mTrianglesToRender)
        {
            DrawFilledTriangle(triangle, triangle.GetColor());
			DrawTriangle(triangle, 0xff0000ff);
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

    bool IsTriangleFrontFaceVisibleToCamera(const glm::vec3& cameraPosition, const std::array<glm::vec4, 3>& transformedVertices)
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
        glm::vec3 normal = glm::cross(vectorAB, vectorAC);
		normal = glm::normalize(normal);

        // Find the vector between vertex A in the triangle and the camera origin
        glm::vec3 cameraRay = cameraPosition - vectorA;
		cameraRay = glm::normalize(cameraRay);        

		float dotNormalCamera = glm::dot(normal, cameraRay);

		return dotNormalCamera > 0.0f;
    }

    void DrawFilledTriangle(const Triangle& triangle, uint32_t color)
    {
        /* 
           Draw a filled triangle using the scanline algorithm by splitting it into 
           flat-top and flat-bottom sections.
           
                      (x0,y0)
                        / \
                       /   \
                      /     \
                     /       \
                    /         \
               (x1,y1)------(Mx,My)  (Mid-point value)
                   \_           \
                      \_         \
                         \_       \
                            \_     \
                               \    \
                                 \_  \
                                    \_\
                                       \
                                     (x2,y2)

         */
	    
        // Some scan lines may not render when using `Vector2f`
        glm::ivec2 point0 = glm::ivec2(triangle.GetPoint(0));
        glm::ivec2 point1 = glm::ivec2(triangle.GetPoint(1));
        glm::ivec2 point2 = glm::ivec2(triangle.GetPoint(2));

        if (point0.y > point1.y) 
        {
			std::swap(point0, point1);
        }
        if (point1.y > point2.y) 
        {
            std::swap(point1, point2);
        }
        if (point0.y > point1.y)
        {
            std::swap(point0, point1);
        }
		        
        // No bottom triangle
        if (point1.y == point2.y)
        {           			
		    FillFlatBottomTriangle(point0, point1, point2, color);
        }
        // No top triangle
        else if (point0.y == point1.y)
        {
            FillFlatTopTriangle(point0, point1, point2, color);
        }
        else
        {
            // Compute mid point value (seperate flat-top and flat-bottom triangles)
            glm::ivec2 m = {
                (((point2.x - point0.x) * (point1.y - point0.y)) / (point2.y - point0.y)) + point0.x,
                point1.y
            };

            FillFlatBottomTriangle(point0, point1, m, color);
            FillFlatTopTriangle(point1, m, point2, color);
        }
    }

	void FillFlatBottomTriangle(const glm::ivec2& point0, const glm::ivec2& point1, const glm::ivec2& point2, uint32_t color)
	{   
        /*
                 (x0,y0)
                   / \
                  /   \
                 /     \
                /       \
               /         \
           (x1,y1)------(x2,y2)
        */

        // y is the independent variable; the slope determines how x changes as y increases.
        float inverseSlope1 = static_cast<float>(point1.x - point0.x) / (point1.y - point0.y);
        float inverseSlope2 = static_cast<float>(point2.x - point0.x) / (point2.y - point0.y);

        // Loop all the scanlines from top to bottom
        float xStart = static_cast<float>(point0.x);
        float xEnd = static_cast<float>(point0.x);

        for (int32_t y = point0.y; y <= point2.y; y++)
        {
            DrawLine(
                static_cast<int32_t>(xStart),
                y,
                static_cast<int32_t>(xEnd),
                y,
                color
            );

            xStart += inverseSlope1;
            xEnd += inverseSlope2;
        }        
	}

    void FillFlatTopTriangle(const glm::ivec2& point0, const glm::ivec2& point1, const glm::ivec2& point2, uint32_t color)
    {
        /*
           (x0,y0)------(x1,y1)
               \         /
                \       /
                 \     /
                  \   /
                   \ /
                 (x2,y2)
        */

        // y is the independent variable; the slope determines how x changes as y increases.
        float inverseSlope1 = static_cast<float>(point2.x - point0.x) / (point2.y - point0.y);
        float inverseSlope2 = static_cast<float>(point2.x - point1.x) / (point2.y - point1.y);

        // Loop all the scanlines from top to bottom
        float xStart = static_cast<float>(point2.x);
        float xEnd = static_cast<float>(point2.x);

        for (int32_t y = point2.y; y >= point0.y; y--)
        {
            DrawLine(
                static_cast<int32_t>(xStart),
                y,
                static_cast<int32_t>(xEnd),
                y,
                color
            );

            xStart -= inverseSlope1;
            xEnd -= inverseSlope2;
        }
    }

    void DrawTriangle(const Triangle& triangle, uint32_t color)
    {
        DrawLine(
            static_cast<int32_t>(triangle.GetPoint(0).x),
            static_cast<int32_t>(triangle.GetPoint(0).y),
            static_cast<int32_t>(triangle.GetPoint(1).x),
            static_cast<int32_t>(triangle.GetPoint(1).y),
            color
        );

        DrawLine(
            static_cast<int32_t>(triangle.GetPoint(1).x),
            static_cast<int32_t>(triangle.GetPoint(1).y),
            static_cast<int32_t>(triangle.GetPoint(2).x),
            static_cast<int32_t>(triangle.GetPoint(2).y),
            color
        );

        DrawLine(
            static_cast<int32_t>(triangle.GetPoint(2).x),
            static_cast<int32_t>(triangle.GetPoint(2).y),
            static_cast<int32_t>(triangle.GetPoint(0).x),
            static_cast<int32_t>(triangle.GetPoint(0).y),
            color
        );
    }

    void DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
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
    DirectionalLight mDirectionalLight;

    float mNear = 2.0f;
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