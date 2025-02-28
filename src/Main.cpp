// Includes
//------------------------------------------------------------------------------
// Application
#include "Mesh.h"
#include "Matrix.h"
#include "Light.h"
#include "Texture.h"
#include "Clipping.h"
#include "ColorBuffer.h"
#include "GeometryRenderer.h"

// Core
#include "Core/AppCore.h"
#include "Core/Utils.h"
#include "Core/SDLWrappers/SDLTexture.h"

// Third party
#include <SDL.h>
#include <glm/glm.hpp>

// System
#include <functional>
#include <chrono>

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
class ZBuffer
{
public:
    ZBuffer(AppContext& context)
        : mBuffer(context.GetWindowSize().x* context.GetWindowSize().y, 1.0f)
		, mSize(context.GetWindowSize())
    { }

    void Clear()
    {
		std::fill(mBuffer.begin(), mBuffer.end(), 1.0f);
    }

    void SetDepth(int32_t x, int32_t y, float depth)
    {   
        if (x >= 0 && x < mSize.x && y >= 0 && y < mSize.y)
        {
            mBuffer[y * mSize.x + x] = depth;
        }		
    }

    float GetDepth(int32_t x, int32_t y) const
    {        
        if (x >= 0 && x < mSize.x && y >= 0 && y < mSize.y)
		{
            return mBuffer[y * mSize.x + x];
		}

		return 1.0f;		
    }

private:
    std::vector<float> mBuffer;
	glm::ivec2 mSize;
};

//------------------------------------------------------------------------------
class Camera
{
public:
	Camera()
		: mPosition(0.0f, 0.0f, 0.0f)
		, mDirection(0.0f, 0.0f, 1.0f)
		, mFowardVelocity(0.0f, 0.0f, 0.0f)
		, mYawAngle(0.0f)
	{ }
	
	// w,a,s,d
	glm::vec3 mPosition;
	glm::vec3 mDirection;           // Direction camera is pointing
	glm::vec3 mFowardVelocity;      // Direction camera is moving
    float mYawAngle;
};

//------------------------------------------------------------------------------
class SimpleRendererApplication : public Application
{
public:
    SimpleRendererApplication(const AppConfig& config)
        : Application(config)
    { }    
};

//------------------------------------------------------------------------------
class RendererApplication : public Application
{
public:
	RendererApplication(const AppConfig& config)
		: Application(config)
		, mDirectionalLight({ 0.0f, -1.0f, 1.0f })
		, mZBuffer(GetContext())
		, mColorBuffer(GetContext(), GetContext().GetWindowSize())
	{ }

    virtual void OnCreate() override
    {
        mMesh = CreateMeshFromOBJFile(ResolveAssetPath("drone.obj"));
		mTrianglesToRender.reserve(mMesh->FaceCount());
		mTexture = std::make_unique<Texture>(ResolveAssetPath("drone.png"));

        const  glm::vec2 windowSize = glm::vec2(GetContext().GetWindowSize());

		const float aspectY = windowSize.y / windowSize.x;
        const float aspectX = windowSize.x / windowSize.y;
		const Angle fovY = Angle::Degrees(60.0f);
		const Angle fovX = Angle::Radians(2.0f * glm::atan(glm::tan(fovY.AsRadians() * 0.5f) * aspectX));
        const float near = 0.2f;
		const float far = 110.0f;

        mProjectionMatrix = CreatePerspectiveProjectionMatrix(
            fovY.AsDegrees(),
            aspectY,
            near,
            far
        );

		mClippingPlanes = ComputePerspectiveFrustrumPlanes(fovX, fovY, near, far);
    }

    virtual void OnEvent(const SDL_Event& event, float timeslice) override
    { 
		if (event.type == SDL_KEYDOWN)
		{			
            if (event.key.keysym.sym == SDLK_UP)
            {
                mCamera.mPosition.y += 3.0f * timeslice;
            }
            else if (event.key.keysym.sym == SDLK_DOWN)
            {
                mCamera.mPosition.y -= 3.0f * timeslice;
            }
            else if (event.key.keysym.sym == SDLK_a)
            {
				mCamera.mYawAngle += 1.0f * timeslice;
            }
            else if (event.key.keysym.sym == SDLK_d)
            {
                mCamera.mYawAngle -= 1.0f * timeslice;
            }
            else if (event.key.keysym.sym == SDLK_w)
            {
				mCamera.mFowardVelocity = mCamera.mDirection * 5.0f * timeslice;
				mCamera.mPosition += mCamera.mFowardVelocity;					
            }
            else if (event.key.keysym.sym == SDLK_s)
            {
                mCamera.mFowardVelocity = mCamera.mDirection * 5.0f * timeslice;
                mCamera.mPosition -= mCamera.mFowardVelocity;
            }
            else if (event.key.keysym.sym == SDLK_f)
            {
				mApplyFillRule = !mApplyFillRule;
				std::cout << "Fill rule: " << (mApplyFillRule ? "ON" : "OFF") << std::endl;
            }
		}
    }

    virtual void OnUpdate(float timelice) override
    {
        (void)timelice; // Unused

        const glm::vec2 windowSize = glm::vec2(GetContext().GetWindowSize());

        mZBuffer.Clear();
        mTrianglesToRender.clear();        

        mMesh->AddRotation({ 0.0f, 1.0f, 0.0f });
        mMesh->SetScale({ 1.0f, 1.0f, 1.0f });
        mMesh->SetTranslation({ 0.0f, 0.0f, 4.0f });

        glm::mat4 modelMatrix = ComputeModelMatrix(*mMesh);

        // Init with z-axis pointing forward
        glm::vec3 target{ 0.0f, 0.0f, 1.0f };
        mCamera.mDirection = CreateRotateAboutYMatrix(glm::degrees(mCamera.mYawAngle)) * glm::vec4(target, 0.0f);

        // Offset the camera position in the direction where the camera is pointing at
        target = mCamera.mPosition + mCamera.mDirection;
        glm::vec3 up { 0.0f, 1.0f, 0.0f };
        glm::mat4 viewMatrix = CreateLookAt(mCamera.mPosition, target, up);

        // Build up a list of projected triangles to render
        for (size_t i = 0; i < mMesh->FaceCount(); i++)
        {            
            const Face& face = mMesh->GetFace(i);
                        
            Triangle triangle;
            
            for (size_t j = 0; j < 3; j++)
            {
                Vertex& vertexData = triangle.mVertices[j];                
                vertexData.mPoint = glm::vec4(mMesh->GetVertex(face.mVertexIndicies[j]), 1.0f);
				vertexData.mPoint = viewMatrix * modelMatrix * vertexData.mPoint;                
				vertexData.mUV = mMesh->GetUV(face.mTextureIndicies[j]);
            }

            glm::vec3 faceNormal = ComputeFaceNormal(triangle);

            glm::ivec3 origin = { 0, 0, 0 };  // LookAt matrix tranforms the origin to the camera position
            if (!IsTriangleFrontFaceVisibleToCamera(origin, faceNormal, triangle))
            {
                continue;
            }
            
            // Clipping (enter with 1 triangle, exit with 0 or more triangles)        
            for (Triangle& clippedTriangle : ClipWithFrustum(mClippingPlanes, triangle))
            {                
                for (size_t j = 0; j < 3; j++)
                {
                    Vertex& vertex = clippedTriangle.mVertices[j];                    
                    
					// Apply perspective division
                    vertex.mPoint = ProjectVec4(mProjectionMatrix, vertex.mPoint);

                    // Negate the Y-coordinate to correct for SDL's inverted Y-coordinate system
                    vertex.mPoint.y *= -1.0f;

                    // Scale into the view
                    vertex.mPoint.x *= windowSize.x * 0.5f;
                    vertex.mPoint.y *= windowSize.y * 0.5f;

                    // Translate the projected points to the middle of the screen
                    vertex.mPoint.x += windowSize.x * 0.5f;
                    vertex.mPoint.y += windowSize.y * 0.5f;                    
                }

				// Apply directional lighting
                float lightIntensity = mDirectionalLight.CalculateLightIntensity(faceNormal);
                uint32_t shadedColor = ApplyLightIntensity(clippedTriangle.mColor, lightIntensity);
                clippedTriangle.mColor = shadedColor;											

                mTrianglesToRender.push_back(clippedTriangle);
            }
        }
    }

    virtual void OnRender() override
    {
		mColorBuffer.Clear(0x00000000);
                		
        auto start = std::chrono::high_resolution_clock::now();
        for (Triangle& triangle : mTrianglesToRender)
        {
            const auto& vertices = triangle.mVertices;

            DrawTexturedTriangle(
                { vertices[0].mPoint, vertices[1].mPoint, vertices[2].mPoint },
                { vertices[0].mUV, vertices[1].mUV, vertices[2].mUV },
                *mTexture);
        }

        auto end = std::chrono::high_resolution_clock::now();        
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "Execution time: " << duration.count() << " ms\n";

		mColorBuffer.Render();
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

    glm::vec3 ComputeFaceNormal(const Triangle& triangle)
    {
        // Check backface culling
        glm::vec3 vectorA = triangle.mVertices[0].mPoint;  /*   A   */
        glm::vec3 vectorB = triangle.mVertices[1].mPoint;  /*  / \  */
        glm::vec3 vectorC = triangle.mVertices[2].mPoint;  /* C---B */

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

    bool IsTriangleFrontFaceVisibleToCamera(const glm::vec3& cameraPosition, const glm::vec3& faceNormal, const Triangle& triangle)
    {
        // Find the vector between vertex A in the triangle and the camera origin
        glm::vec3 cameraRay = cameraPosition - glm::vec3(triangle.mVertices[0].mPoint);
		cameraRay = glm::normalize(cameraRay);        

		float dotNormalCamera = glm::dot(faceNormal, cameraRay);

		return dotNormalCamera > 0.0f;
    }     

    void DrawTexturedTriangle(const std::array<glm::vec4, 3>& vertices, const std::array<glm::vec2, 3>& uvs,
                              const Texture& texture)
    {
        // Vertex positions (integer screen coordinates)
        glm::ivec2 p0 = vertices[0];
        glm::ivec2 p1 = vertices[1];
        glm::ivec2 p2 = vertices[2];

        // Precompute inverse depth for perspective-correct interpolation
        float invW0 = 1.0f / vertices[0].w;
        float invW1 = 1.0f / vertices[1].w;
        float invW2 = 1.0f / vertices[2].w;

        // Texture coordinates for each vertex
        glm::vec2 uv0 = uvs[0];
        glm::vec2 uv1 = uvs[1];
        glm::vec2 uv2 = uvs[2];

        // Compute inverse area for barycentric interpolation
        float invTriangleArea = 1.0f / static_cast<float>(EdgeCrossProduct(p0, p1, p2));

        // Compute bounding box
        int32_t xMin = std::min({ p0.x, p1.x, p2.x });
        int32_t yMin = std::min({ p0.y, p1.y, p2.y });
        int32_t xMax = std::max({ p0.x, p1.x, p2.x });
        int32_t yMax = std::max({ p0.y, p1.y, p2.y });

        // Precompute edge function step deltas for rasterization
        int deltaEdge0X = (p1.y - p2.y);
        int deltaEdge1X = (p2.y - p0.y);
        int deltaEdge2X = (p0.y - p1.y);
        int deltaEdge0Y = (p2.x - p1.x);
        int deltaEdge1Y = (p0.x - p2.x);
        int deltaEdge2Y = (p1.x - p0.x);

        // Compute edge function values for the top-left pixel
        glm::ivec2 topLeftPixel = { xMin, yMin };
        int32_t edge0 = EdgeCrossProduct(p1, p2, topLeftPixel);
        int32_t edge1 = EdgeCrossProduct(p2, p0, topLeftPixel);
        int32_t edge2 = EdgeCrossProduct(p0, p1, topLeftPixel);

        // Texture size for coordinate clamping
        const glm::ivec2 texSize = texture.GetSize();        

        // Loop over the bounding box (rasterization)
        for (int32_t y = yMin; y < yMax; y++)
        {
            int32_t e0 = edge0;
            int32_t e1 = edge1;
            int32_t e2 = edge2;

            for (int32_t x = xMin; x < xMax; x++)
            {
                // Check if the pixel is inside the triangle
                if (e0 >= 0 && e1 >= 0 && e2 >= 0)
                {
                    // Compute barycentric weights
                    float alpha = e0 * invTriangleArea;
                    float beta = e1 * invTriangleArea;
                    float gamma = e2 * invTriangleArea;

                    // Perspective-correct depth interpolation
                    float interpolatedInvW = alpha * invW0 + beta * invW1 + gamma * invW2;
                    float depth = 1.0f - interpolatedInvW;

                    // Z-buffer test
                    float currentDepth = mZBuffer.GetDepth(x, y);
                    if (depth < currentDepth)
                    {
                        // Perspective-correct UV interpolation
                        float u = (alpha * (uv0.x * invW0) + beta * (uv1.x * invW1) + gamma * (uv2.x * invW2)) / interpolatedInvW;
                        float v = (alpha * (uv0.y * invW0) + beta * (uv1.y * invW1) + gamma * (uv2.y * invW2)) / interpolatedInvW;

                        // Convert UV to texture coordinates (modulo for wrapping)
                        int32_t texX = static_cast<int32_t>(u * (texSize.x - 1)) % texSize.x;
                        int32_t texY = static_cast<int32_t>(v * (texSize.y - 1)) % texSize.y;
                        if (texX < 0) texX += texSize.x;
                        if (texY < 0) texY += texSize.y;

                        // Fetch texel color and render pixel
						uint32_t color = texture.GetPixel(texX, texY);
                        mColorBuffer.SetPixel(x, y, color);
                        mZBuffer.SetDepth(x, y, depth);
                    }
                }

                // Step edge functions in X direction
                e0 += deltaEdge0X;
                e1 += deltaEdge1X;
                e2 += deltaEdge2X;
            }

            // Step edge functions in Y direction
            edge0 += deltaEdge0Y;
            edge1 += deltaEdge1Y;
            edge2 += deltaEdge2Y;
        }
    }

    int32_t EdgeCrossProduct(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2 point)
    {
		return (b.x - a.x) * (point.y - a.y) - (b.y - a.y) * (point.x - a.x);
    }

	bool IsFlatTopOrLeftEdge(const glm::ivec2& start, const glm::ivec2& end)
	{
		glm::ivec2 edge = end - start;		
        bool isFlatTopEdge = (edge.y == 0 && edge.x > 0);
		bool isLeftEdge = (edge.y < 0);

		return isFlatTopEdge || isLeftEdge;
	}

	std::vector<Triangle> mTrianglesToRender;
	std::unique_ptr<Mesh> mMesh;
    std::unique_ptr<Texture> mTexture;
    DirectionalLight mDirectionalLight;
	
	ColorBuffer mColorBuffer;
    ZBuffer mZBuffer;
	
    Camera mCamera;
    glm::mat4 mProjectionMatrix;
	std::array<Plane, 6> mClippingPlanes;
    bool mApplyFillRule = false;
};

//------------------------------------------------------------------------------
std::unique_ptr<Application> CreateApplication()
{
	AppConfig config;
	config.mWindowTitle = "My Custom SDL App";
	config.mFullscreen = false;
	config.mUseNativeResolution = false;
	config.mMonitorIndex = 1;
	config.mWindowSize = { 800, 600 };

	return std::make_unique<RendererApplication>(config);
}