#pragma once

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include "Camera.h"
#include <vector>

struct GLFWwindow;

enum ShapeType
{
	SHAPE_SQUARE,
	SHAPE_CIRCLE,
	SHAPE_NONE,
};

class Graphics
{
private:
	void SetShaderUniforms(glm::vec3 inPosition, glm::vec3 inScale, glm::vec3 inColor);
	ShapeType shapeTypeForBatch;

private:
	static glm::vec2 displaySize;
	static float aspectRatio;

public:
	static glm::ivec2 GetDisplaySize() { return displaySize; }
	static float GetAspectRatio() { return aspectRatio; }
	static void UpdateDisplaySize(int displayWidth, int displayHeight);

public:


	// single color shader
	GLuint singleColorShaderId;
	GLuint mvpId;
	GLuint modelId;
	GLuint colorId;

	// texture shader
	GLuint textureShaderId;
	GLuint textureMvpId;

	GLuint squareVertexBuffer;
	GLuint circleVertexBuffer;
	GLuint lineVertexBuffer;

	GLuint texturebuffer;
	GLuint renderbuffer;
	GLuint framebuffer;

	GLuint vertexarray;

	Camera* camera;

	bool isBatchActive;

	GLFWwindow* window;
public:
	Graphics();
	~Graphics();

	bool initGraphics();
	void CleanUp();
	void DrawTexture();
	void LineBatchBegin();
	void LineBatchEnd();

	void DrawLine(const std::vector<glm::vec3> points, glm::vec3 inColor = glm::vec3(1.f), float lineWidth = 1.f);
	void DrawBatch(glm::vec3 inPosition, glm::vec3 inColor = glm::vec3(1.f), glm::vec3 inScale = glm::vec3(1.f));
	void ShapeBatchBegin(ShapeType inShape);
	void ShapeBatchEnd();
};

