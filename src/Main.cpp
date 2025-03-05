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
#include "TriangleRasterizer.h"
#include "ZBuffer.h"

// Core
#include "Core/AppCore.h"
#include "Core/Utils.h"
#include "Core/SDLWrappers/SDLTexture.h"

// Third party
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

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
        , mColorBuffer(GetContext(), { 128, 128 })
    { }

    virtual void OnEvent(const SDL_Event& event, float timeslice)
    {
        (void)timeslice; // Unused

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_1)
            {
                mDrawTriangle0 = !mDrawTriangle0;
            }
            else if (event.key.keysym.sym == SDLK_2)
            {
                mDrawTriangle1 = !mDrawTriangle1;
            }
        }
    }

    virtual void OnRender() override
    {
        static std::vector<glm::vec2> vertices = {
            { 40, 40 },
            { 80, 40 },
            { 40, 80 },
            { 90, 90 }
        };

        static std::vector<glm::vec3> colors = {
            {0xFF, 0x00, 0x00 },
            {0x00, 0xFF, 0x00 },
            {0x00, 0x00, 0xFF }
        };

        float angle = static_cast<float>(SDL_GetTicks() / 1000.0f * 0.1f);
        glm::vec2 center = { 60, 60 };

        glm::vec2 v0 = Rotate(vertices[0], center, angle);
        glm::vec2 v1 = Rotate(vertices[1], center, angle);
        glm::vec2 v2 = Rotate(vertices[2], center, angle);
        glm::vec2 v3 = Rotate(vertices[3], center, angle);

        mColorBuffer.Clear(0x00000000);
        if (mDrawTriangle0)
        {
            DrawFilledTriangle(
                { v0, v1, v2 },
                { colors[0], colors[0], colors[0] }
            );
        }
        if (mDrawTriangle1)
        {
            DrawFilledTriangle(
                { v3, v2, v1 },
                { colors[1], colors[1], colors[1] }
            );
        }
        mColorBuffer.Render();
    }

private:
    void DrawFilledTriangle(const std::array<glm::vec2, 3>& vertices, const std::array<glm::vec3, 3>& colors)
    {
        // Vertex positions (integer screen coordinates)
        glm::vec2 p0 = vertices[0];
        glm::vec2 p1 = vertices[1];
        glm::vec2 p2 = vertices[2];

        // Compute inverse area for barycentric interpolation
        float invTriangleArea = 1.0f / static_cast<float>(EdgeCrossProduct(p0, p1, p2));

        // Compute bounding box
        int32_t xMin = static_cast<int32_t>(std::ceil(std::min({ p0.x, p1.x, p2.x })));
        int32_t yMin = static_cast<int32_t>(std::ceil(std::min({ p0.y, p1.y, p2.y })));
        int32_t xMax = static_cast<int32_t>(std::ceil(std::max({ p0.x, p1.x, p2.x })));
        int32_t yMax = static_cast<int32_t>(std::ceil(std::max({ p0.y, p1.y, p2.y })));

        // Precompute edge function step deltas for rasterization
        float deltaEdge0X = (p1.y - p2.y);
        float deltaEdge1X = (p2.y - p0.y);
        float deltaEdge2X = (p0.y - p1.y);
        float deltaEdge0Y = (p2.x - p1.x);
        float deltaEdge1Y = (p0.x - p2.x);
        float deltaEdge2Y = (p1.x - p0.x);

        // Compute edge function values for the top-left pixel
        glm::ivec2 topLeftPixel = { xMin + 0.5f, yMin + 0.5f };
        float edge0 = EdgeCrossProduct(p1, p2, topLeftPixel);
        float edge1 = EdgeCrossProduct(p2, p0, topLeftPixel);
        float edge2 = EdgeCrossProduct(p0, p1, topLeftPixel);

        // Loop over the bounding box (rasterization)
        for (int32_t y = yMin; y < yMax; y++)
        {
            float e0 = edge0;
            float e1 = edge1;
            float e2 = edge2;

            for (int32_t x = xMin; x < xMax; x++)
            {
                // Check if the pixel is inside the triangle
                if (e0 >= 0 && e1 >= 0 && e2 >= 0)
                {
                    // Compute barycentric weights
                    float alpha = e0 * invTriangleArea;
                    float beta = e1 * invTriangleArea;
                    float gamma = e2 * invTriangleArea;

                    // Interpolate color                    
                    uint32_t a = 0xff;
                    uint32_t r = static_cast<uint32_t>(alpha * colors[0].r + beta * colors[1].r + gamma * colors[2].r);
                    uint32_t g = static_cast<uint32_t>(alpha * colors[0].g + beta * colors[1].g + gamma * colors[2].g);
                    uint32_t b = static_cast<uint32_t>(alpha * colors[0].b + beta * colors[1].b + gamma * colors[2].b);

                    uint32_t interpolatedColor = 0x00000000;
                    interpolatedColor = (interpolatedColor | a) << 8;
                    interpolatedColor = (interpolatedColor | b) << 8;
                    interpolatedColor = (interpolatedColor | g) << 8;
                    interpolatedColor = (interpolatedColor | r);

                    mColorBuffer.SetPixel(x, y, interpolatedColor);
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

    glm::vec4 Rotate(glm::vec2 v, glm::vec2 center, float angle)
    {
        glm::vec4 rot;
        v.x -= center.x;
        v.y -= center.y;
        rot.x = v.x * cos(angle) - v.y * sin(angle);
        rot.y = v.x * sin(angle) + v.y * cos(angle);
        rot.x += center.x;
        rot.y += center.y;

        return rot;
    }

    float EdgeCrossProduct(const glm::vec2& a, const glm::vec2& b, const glm::vec2 point)
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

    ColorBuffer mColorBuffer;
    bool mDrawTriangle0 = false;
    bool mDrawTriangle1 = false;
};

//------------------------------------------------------------------------------
struct LineSegment
{
	glm::ivec2 mStart;
	glm::ivec2 mEnd;
};

//------------------------------------------------------------------------------
struct Transform
{
	glm::vec3 mRotation;
    glm::vec3 mScale;
	glm::vec3 mTranslation;
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
        mMesh = CreateMeshFromOBJFile(ResolveAssetPath("cube.obj"));
		mTrianglesToRender.reserve(mMesh->FaceCount());
		mTexture = std::make_unique<Texture>(ResolveAssetPath("cube.png"));

        const glm::vec2 windowSize = glm::vec2(GetContext().GetWindowSize());

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

    std::vector<Triangle> TransformMeshToScreen(const Mesh& mesh, const Transform& transform, glm::mat4& view, const glm::vec2& windowSize)
    {
        std::vector<Triangle> trianglesToRender;

        const glm::mat4 model = ComputeModelMatrix(transform);

        for (size_t i = 0; i < mesh.FaceCount(); i++)
        {            
            Triangle triangle = FaceToTriangle(*mMesh, mMesh->GetFace(i));

            for (size_t j = 0; j < 3; j++)
            {
                Vertex& vertex = triangle.mVertices[j];
                vertex.mPoint = view * model * vertex.mPoint;
                vertex.mPoint = TransformPointFromViewToScreen(windowSize, mProjectionMatrix, vertex.mPoint);
            }

			trianglesToRender.push_back(triangle);
        }

		return trianglesToRender;
    }

    virtual void OnUpdate(float timelice) override
    {
        (void)timelice; // Unused

        const glm::vec2 windowSize = glm::vec2(GetContext().GetWindowSize());

        mZBuffer.Clear();
        mTrianglesToRender.clear();        
		mLineSegments.clear();

		static Transform transform;
		transform.mRotation.y += 1.0f;
		transform.mScale = { 1.0f, 1.0f, 1.0f };
		transform.mTranslation = { 0.0f, 0.0f, 4.0f };

        glm::mat4 modelMatrix = ComputeModelMatrix(transform);

        // Init with z-axis pointing forward
        glm::vec3 target{ 0.0f, 0.0f, 1.0f };
        mCamera.mDirection = CreateRotateAboutYMatrix(glm::degrees(mCamera.mYawAngle)) * glm::vec4(target, 0.0f);

        // Offset the camera position in the direction where the camera is pointing at
        target = mCamera.mPosition + mCamera.mDirection;
        glm::vec3 up { 0.0f, 1.0f, 0.0f };
        glm::mat4 viewMatrix = CreateLookAt(mCamera.mPosition, target, up);

		// Pre-process normals for smooth shading
		std::unordered_map<size_t, std::vector<Face>> sharedVertexFaces;

        Transform newTransform = transform;
		newTransform.mScale *= 1.5f;
		mWireframeTrianglesToRender = TransformMeshToScreen(*mMesh, newTransform, viewMatrix, windowSize);
        
        for (size_t i = 0; i < mMesh->FaceCount(); i++)
        {
			const Face& face = mMesh->GetFace(i);

			for (size_t j = 0; j < 3; j++)
			{
                int32_t index = face.mVertexIndicies[j];
				sharedVertexFaces[index].push_back(face);
			}
        }

        for (const auto& [vertexIndex, faces] : sharedVertexFaces)
        {
            // Ensure each vertex index maps to at least one face
            assert(!faces.empty() && "Error: Vertex index must be shared by at least one face!");

            std::cout << "Vertex Index: " << vertexIndex << " is shared by faces: ";

            for (const Face& face : faces)
            {
                // Ensure that the vertex actually exists in the face
                assert((face.mVertexIndicies[0] == vertexIndex ||
                    face.mVertexIndicies[1] == vertexIndex ||
                    face.mVertexIndicies[2] == vertexIndex) &&
                    "Error: Face does not actually contain this vertex!");

                std::cout << "(" << face.mVertexIndicies[0] << ", "
                    << face.mVertexIndicies[1] << ", "
                    << face.mVertexIndicies[2] << ") ";
            }
            std::cout << std::endl;
        }

        // Build up a list of projected triangles to render
        for (size_t i = 0; i < mMesh->FaceCount(); i++)
        {            
            const Face& face = mMesh->GetFace(i);                        
            Triangle triangle = FaceToTriangle(*mMesh, mMesh->GetFace(i));
            
            for (size_t j = 0; j < 3; j++)
            {
                Vertex& vertexData = triangle.mVertices[j];
				vertexData.mPoint = viewMatrix * modelMatrix * vertexData.mPoint;
				
				// Average the normals of the shared vertices for smooth shading
				const size_t vertexIndex = face.mVertexIndicies[j];                
                
                glm::vec3 averagedNormal { };
				for (const Face& sharedFace : sharedVertexFaces[vertexIndex])
				{
                    Triangle sharedTriangle = FaceToTriangle(*mMesh, sharedFace);
                    averagedNormal += glm::normalize(ComputeFaceNormal(sharedTriangle));
				}
				averagedNormal = glm::normalize(averagedNormal);				
                vertexData.mNormal = TransformNormal(modelMatrix, viewMatrix, averagedNormal);
				
                //vertexData.mNormal = mMesh->GetNormal(face.mNormalIndicies[j]);
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
                    
                    glm::vec3 start = vertex.mPoint;
                    glm::vec3 end = start + vertex.mNormal;

                    LineSegment lineSegment {
                        TransformPointFromViewToScreen(windowSize, mProjectionMatrix, start),
                        TransformPointFromViewToScreen(windowSize, mProjectionMatrix, end)
                    };
					mLineSegments.push_back(lineSegment);					

                    vertex.mPoint = TransformPointFromViewToScreen(windowSize, mProjectionMatrix, vertex.mPoint);                    
                }

				// Apply directional lighting
                float lightIntensity = mDirectionalLight.CalculateLightIntensity(faceNormal);
                uint32_t shadedColor = ApplyLightIntensity(clippedTriangle.mColor, lightIntensity);
                clippedTriangle.mColor = shadedColor;											

                mTrianglesToRender.push_back(clippedTriangle);
            }
        }
    }

    Triangle FaceToTriangle(const Mesh& mesh, const Face& face)
    {
        Triangle triangle;

        for (size_t j = 0; j < 3; j++)
        {
            Vertex& vertexData = triangle.mVertices[j];
            
			vertexData.mPoint = glm::vec4(mesh.GetVertex(face.mVertexIndicies[j]), 1.0f);
			vertexData.mUV = mesh.GetUV(face.mTextureIndicies[j]);
            vertexData.mNormal = mesh.GetNormal(face.mNormalIndicies[j]);
        }

		return triangle;
    }

    virtual void OnRender() override
    {
		mColorBuffer.Clear(0x00000000);

        auto start = std::chrono::high_resolution_clock::now();
        for (Triangle& triangle : mTrianglesToRender)
        {
            const auto& vertices = triangle.mVertices;

            DrawTexturedTriangle(mColorBuffer, mZBuffer,
                { vertices[0].mPoint, vertices[1].mPoint, vertices[2].mPoint },
                { vertices[0].mUV, vertices[1].mUV, vertices[2].mUV },
                { 1.0f, 1.0f, 1.0f },
                *mTexture);

			//DrawWireframeTriangle(mColorBuffer,
			//	{ vertices[0].mPoint, vertices[1].mPoint, vertices[2].mPoint },
			//	0xFFFFFFFF);
        }

		for (Triangle& triangle : mWireframeTrianglesToRender)
		{
			const auto& vertices = triangle.mVertices;

			DrawWireframeTriangle(mColorBuffer,
				{ vertices[0].mPoint, vertices[1].mPoint, vertices[2].mPoint },
				0xFFFFFFFF);
		}

		for (const LineSegment& lineSegment : mLineSegments)
		{
			DrawLine(mColorBuffer, lineSegment.mStart, lineSegment.mEnd, 0xFF000000);
		}

        auto end = std::chrono::high_resolution_clock::now();        
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "Execution time: " << duration.count() << " ms\n";

		mColorBuffer.Render();
    }

private:
    glm::vec3 TransformNormal(const glm::mat4& model, const glm::mat4& view, const glm::vec3 normal)
    {
        const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(view * model)));
        const glm::vec3 transformedNormal = glm::normalize(normalMatrix * normal);

        return transformedNormal;
    }

    glm::vec4 TransformPointFromViewToScreen(const glm::vec2& windowSize, const glm::mat4& projection, const glm::vec3& point)
    {
		return TransformPointFromViewToScreen(windowSize, projection, glm::vec4(point, 1.0f));
    }

    glm::vec4 TransformPointFromViewToScreen(const glm::vec2& windowSize, const glm::mat4& projection, const glm::vec4& point)
    {        
        // Apply perspective division
        glm::vec4 transformedPoint = ProjectVec4(projection, point);

        // Negate the Y-coordinate to correct for SDL's inverted Y-coordinate system
        transformedPoint.y *= -1.0f;

        // Scale into the view
        transformedPoint.x *= windowSize.x * 0.5f;
        transformedPoint.y *= windowSize.y * 0.5f;

        // Translate the projected points to the middle of the screen
        transformedPoint.x += windowSize.x * 0.5f;
        transformedPoint.y += windowSize.y * 0.5f;

		return transformedPoint;
    }

    glm::mat4 ComputeModelMatrix(const Transform& transform)
    {
		glm::mat4 model = glm::mat4(1.0f);        

		model *= CreateTranslationMatrix(transform.mTranslation);   // Applied 5th
	    model *= CreateRotateAboutZMatrix(transform.mRotation.z);   // Applied 4th
		model *= CreateRotateAboutYMatrix(transform.mRotation.y);   // Applied 3rd
        model *= CreateRotateAboutXMatrix(transform.mRotation.x);   // Applied 2nd  
		model *= CreateScaleMatrix(transform.mScale);               // Applied 1st

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
    std::vector<Triangle> mWireframeTrianglesToRender;
    
    std::unique_ptr<Mesh> mMesh;
    std::unique_ptr<Texture> mTexture;
    DirectionalLight mDirectionalLight;
	
	ColorBuffer mColorBuffer;
    ZBuffer mZBuffer;
	
    Camera mCamera;
    glm::mat4 mProjectionMatrix;
	std::array<Plane, 6> mClippingPlanes;
    bool mApplyFillRule = false;
	std::vector<LineSegment> mLineSegments;
};

//------------------------------------------------------------------------------
std::unique_ptr<Application> CreateApplication()
{
	AppConfig config;
	config.mWindowTitle = "My Custom SDL App";
	config.mFullscreen = false;
	config.mUseNativeResolution = false;
	config.mMonitorIndex = 1;
	config.mWindowSize = { 800, 800 };

	return std::make_unique<RendererApplication>(config);
}