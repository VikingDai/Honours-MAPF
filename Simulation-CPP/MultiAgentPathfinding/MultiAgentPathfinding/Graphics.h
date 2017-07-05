#pragma once

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include "Camera.h"

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


public:
	// Create and compile our GLSL program from the shaders
	GLuint singleColorShaderId;
	GLuint mvpId;
	GLuint modelId;
	GLuint colorId;

	GLuint squareVertexBuffer;
	GLuint circleVertexBuffer;

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
	void DrawSquare(glm::vec3 inPosition, glm::vec3 inColor = glm::vec3(1.f), glm::vec3 inScale = glm::vec3(1.f));
	void DrawBatch(glm::vec3 inPosition, glm::vec3 inColor = glm::vec3(1.f), glm::vec3 inScale = glm::vec3(1.f));
	void ShapeBatchBegin(ShapeType inShape);
	void ShapeBatchEnd();
};

