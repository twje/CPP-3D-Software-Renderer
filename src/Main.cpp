// Includes
//------------------------------------------------------------------------------
// Application
#include "Mesh.h"
#include "Matrix.h"
#include "Light.h"
#include "Texture.h"
#include "Clipping.h"

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
class RendererApplication : public Application
{
public:
	RendererApplication(const AppConfig& config)
		: Application(config)
		, mDirectionalLight({ 0.0f, -1.0f, 1.0f })
		, mZBuffer(GetContext())
	{ }

    virtual void OnCreate() override
    {
		mPixelRenderer = std::make_unique<PixelRenderer>(GetContext());

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
        mPixelRenderer->Clear(0x00000000);
        
        std::array<uint32_t, 3> colors{ 0xFF0000FF, 0x00FF00FF, 0x0000FFFF };
        size_t colorCount = 0;
		
        auto start = std::chrono::high_resolution_clock::now();
        for (Triangle& triangle : mTrianglesToRender)
        {
			triangle.mColor = colors[colorCount++ % colors.size()];

            bool drawTextured = true;
			bool drawFlatShaded = false;

            if (drawTextured)
            {
                DrawTexturedTriangleV2(triangle, *mTexture);
            }

            if (drawFlatShaded)
            {
				DrawFilledTriangle(triangle);
            }

            if (!drawTextured && !drawFlatShaded)
            {
				DrawTriangleWireframe(triangle, 0xFFFFFFFF);
                
                // Draw a small red dot for each vertex
                glm::ivec4 point0 = triangle.mVertices[0].mPoint;
                glm::ivec4 point1 = triangle.mVertices[1].mPoint;
                glm::ivec4 point2 = triangle.mVertices[2].mPoint;

                DrawRectangle(point0.x - 1, point0.y - 1, 3, 3, 0xFF0000FF);
                DrawRectangle(point1.x - 1, point1.y - 1, 3, 3, 0xFF0000FF);
                DrawRectangle(point2.x - 1, point2.y - 1, 3, 3, 0xFF0000FF);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << "Execution time: " << duration.count() << " ms\n";

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
      
	void DrawTriangleWireframe(const Triangle& triangle, uint32_t color)
	{
		DrawLine(triangle.mVertices[0].mPoint, triangle.mVertices[1].mPoint, color);
		DrawLine(triangle.mVertices[1].mPoint, triangle.mVertices[2].mPoint, color);
		DrawLine(triangle.mVertices[2].mPoint, triangle.mVertices[0].mPoint, color);
	}

    void DrawTexturedTriangle(const Triangle& triangle, const Texture& texture)
    {		
		RasterTriangle(triangle, [&](const glm::ivec2& point, const std::array<Vertex, 3>& vertices)
		{
            DrawTexel(point, vertices, texture);
		});
    }

    void DrawTexturedTriangleV2(const Triangle& triangle, const Texture& texture)
    {
		// For convenience
		glm::ivec2 v0 = triangle.mVertices[0].mPoint;
		glm::ivec2 v1 = triangle.mVertices[1].mPoint;
		glm::ivec2 v2 = triangle.mVertices[2].mPoint;

        // Compute perspective-correct interpolation of UV coordinates
        float invZ0 = 1.0f / triangle.mVertices[0].mPoint.w;
        float invZ1 = 1.0f / triangle.mVertices[1].mPoint.w;
        float invZ2 = 1.0f / triangle.mVertices[2].mPoint.w;

        glm::vec2 uv0 = triangle.mVertices[0].mUV;
        glm::vec2 uv1 = triangle.mVertices[1].mUV;
        glm::vec2 uv2 = triangle.mVertices[2].mUV;

        // Compute the area of the entire triangle/parallelogram
        float area = static_cast<float>(EdgeCrossProduct(v0, v1, v2));

        // Finds the bounding box with all candidate pixels
		int32_t xMin = std::min({ v0.x, v1.x, v2.x });
		int32_t yMin = std::min({ v0.y, v1.y, v2.y });
		int32_t xMax = std::max({ v0.x, v1.x, v2.x });
		int32_t yMax = std::max({ v0.y, v1.y, v2.y });

        // Compute the constant delta_s that will be used for the horizontal and vertical steps
        int deltaW0Col = (v1.y - v2.y);
        int deltaW1Col = (v2.y - v0.y);
        int deltaW2Col = (v0.y - v1.y);
        int deltaW0Row = (v2.x - v1.x);
        int deltaW1Row = (v0.x - v2.x);
        int deltaW2Row = (v1.x - v0.x);

        // Fill convention: Points on a flat top or left edge are inside the triangle. (test first)
        // https://fgiesen.wordpress.com/2013/02/06/the-barycentric-conspirac/
        //int32_t bias0 = IsFlatTopOrLeftEdge(v1, v2) ? 0 : -1;
        //int32_t bias1 = IsFlatTopOrLeftEdge(v2, v0) ? 0 : -1;
        //int32_t bias2 = IsFlatTopOrLeftEdge(v0, v1) ? 0 : -1;
        
        // Compute the edge functions for the fist (top-left) point
		glm::ivec2 point = { xMin, yMin };
        int32_t w0Row = EdgeCrossProduct(v1, v2, point);
        int32_t w1Row = EdgeCrossProduct(v2, v0, point);
        int32_t w2Row = EdgeCrossProduct(v0, v1, point);

		for (int32_t y = yMin; y < yMax; y++)
		{
			int32_t w0 = w0Row;
			int32_t w1 = w1Row;
			int32_t w2 = w2Row;
			for (int32_t x = xMin; x < xMax; x++)
			{
				bool isInside = w0 >= 0 && w1 >= 0 && w2 >= 0;
				if (isInside)
				{
                    // Compute the barycentric coordinates
                    float alpha = w0 / area;
                    float beta = w1 / area;
                    float gamma = w2 / area;

					// Perform perspective-correct interpolation of UV coordinates
                    float interpolatedU = alpha * (uv0.x * invZ0) + beta * (uv1.x * invZ1) + gamma * (uv2.x * invZ2);
                    float interpolatedV = alpha * (uv0.y * invZ0) + beta * (uv1.y * invZ1) + gamma * (uv2.y * invZ2);
 
                    float interpolatedReciprocalZ = alpha * invZ0 + beta * invZ1 + gamma * invZ2;

                    interpolatedU /= interpolatedReciprocalZ;
                    interpolatedV /= interpolatedReciprocalZ;

                    const glm::ivec2 texSize = texture.GetSize();

                    const int32_t texX = std::abs(static_cast<int32_t>(interpolatedU * (texSize.x))) % texSize.x;
                    const int32_t texY = std::abs(static_cast<int32_t>(interpolatedV * (texSize.y))) % texSize.y;

					// Z-buffering
                    if ((1.0f - interpolatedReciprocalZ) < mZBuffer.GetDepth(x, y))
                    {
                        auto color = texture.GetPixel(texX, texY);
                        mPixelRenderer->SetPixel(x, y, color);
                        mZBuffer.SetDepth(x, y, 1.0f - interpolatedReciprocalZ);
                    }
				}

                w0 += deltaW0Col;
                w1 += deltaW1Col;
                w2 += deltaW2Col;
			}
            w0Row += deltaW0Row;
            w1Row += deltaW1Row;
            w2Row += deltaW2Row;
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

	void DrawFilledTriangle(const Triangle& triangle)
	{
		RasterTriangle(triangle, [&](const glm::ivec2& point, const std::array<Vertex, 3>& vertices)
		{
			(void)vertices;
			mPixelRenderer->SetPixel(point.x, point.y, triangle.mColor);
		});
	}

    void RasterTriangle(const Triangle& triangle, const RasterTriangleFunc& callback)
    {
        // Extract triangle vertices
        std::array<Vertex, 3> vertices { triangle.mVertices[0], triangle.mVertices[1], triangle.mVertices[2] };

        // Sort vertices by y-coordinate (ascending)
        std::sort(std::begin(vertices), std::end(vertices), [](const Vertex& a, const Vertex& b) {
            return a.mPoint.y < b.mPoint.y;
        });

        // Use integer points to prevent gaps in scanlines
        const glm::ivec2& point0 = vertices[0].mPoint;
        const glm::ivec2& point1 = vertices[1].mPoint;
        const glm::ivec2& point2 = vertices[2].mPoint;

        // Lambda to fill scanlines
        auto fillScanlines = [&](const glm::ivec2& startPoint, const glm::ivec2& endPoint, float shortSlope, float longSlope) {
            if (endPoint.y - startPoint.y != 0)
            {
                for (int32_t y = startPoint.y; y <= endPoint.y; y++)  // Short edge
                {
                    // Iterate over the short edge
                    int32_t xStart = static_cast<int32_t>(startPoint.x + (y - startPoint.y) * shortSlope);

                    // Iterate over the long edge (point0 is the top most vertex)
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
        };

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
        
        float shortSlope = (point1.y - point0.y == 0)
            ? 0.0f
            : static_cast<float>(point1.x - point0.x) / (point1.y - point0.y);

        fillScanlines(point0, point1, shortSlope, longSlope);

        /*
           (x0,y0)------(x1,y1)
               \         /
                \       /
                 \     /
                  \   /
                   \ /
                 (x2,y2)
        */
        shortSlope = (point2.y - point1.y == 0)
           ? 0.0f
           : static_cast<float>(point2.x - point1.x) / (point2.y - point1.y);

        fillScanlines(point1, point2, shortSlope, longSlope);
    }    

    void DrawTexel(const glm::ivec2& point, const std::array<Vertex, 3>& vertices, const Texture& texture)
    {
        const glm::vec3& weights = BaryCentricWeights(point, vertices);
        
        const float alpha = weights.x;
        const float beta = weights.y;
        const float gamma = weights.z;

        const glm::vec2& uv0 = vertices[0].mUV;
        const glm::vec2& uv1 = vertices[1].mUV; 
        const glm::vec2& uv2 = vertices[2].mUV;
                
        // Perform perspective-correct interpolation of UV coordinates
        const float invW0 = 1.0f / vertices[0].mPoint.w;
        const float invW1 = 1.0f / vertices[1].mPoint.w;
        const float invW2 = 1.0f / vertices[2].mPoint.w;

        float interpolatedU = alpha * (uv0.x * invW0) + beta * (uv1.x * invW1) + gamma * (uv2.x * invW2);
        float interpolatedV = alpha * (uv0.y * invW0) + beta * (uv1.y * invW1) + gamma * (uv2.y * invW2);
        
        float interpolatedReciprocalW = alpha * invW0 + beta * invW1 + gamma * invW2;

        interpolatedU /= interpolatedReciprocalW;
        interpolatedV /= interpolatedReciprocalW;

        const glm::ivec2 texSize = texture.GetSize();

        const int32_t texX = std::abs(static_cast<int32_t>(interpolatedU * (texSize.x - 1))) % texSize.x;
        const int32_t texY = std::abs(static_cast<int32_t>(interpolatedV * (texSize.y - 1))) % texSize.y;

        if ((1.0f - interpolatedReciprocalW) < mZBuffer.GetDepth(point.x, point.y))
        {
            mPixelRenderer->SetPixel(point.x, point.y, texture.GetPixel(texX, texY));
			mZBuffer.SetDepth(point.x, point.y, 1.0f - interpolatedReciprocalW);
        }
    }

    glm::vec3 BaryCentricWeights(const glm::ivec2& point, const std::array<Vertex, 3>& vertices)
    {
		glm::vec2 a = vertices[0].mPoint;
		glm::vec2 b = vertices[1].mPoint;
		glm::vec2 c = vertices[2].mPoint;
		glm::vec2 p = point;

        // Find the vectors between the vertices ABC and point p
        glm::vec2 ac = c - a;
        glm::vec2 ab = b - a;
        glm::vec2 ap = p - a;
        glm::vec2 pc = c - p;
        glm::vec2 pb = b - p;

        // Compute the area of the full parallegram/triangle ABC using 2D cross product
        float area_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x); // || AC x AB ||

        // Alpha is the area of the small parallelogram/triangle PBC divided by the area of the full parallelogram/triangle ABC
        float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc;

        // Beta is the area of the small parallelogram/triangle APC divided by the area of the full parallelogram/triangle ABC
        float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc;

        // Weight gamma is easily found since barycentric coordinates always add up to 1.0
        float gamma = 1 - alpha - beta;

        glm::vec3 weights = { alpha, beta, gamma };
        
        return weights;
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
	config.mFullscreen = true;
	config.mUseNativeResolution = true;
	config.mMonitorIndex = 1;

	return std::make_unique<RendererApplication>(config);
}