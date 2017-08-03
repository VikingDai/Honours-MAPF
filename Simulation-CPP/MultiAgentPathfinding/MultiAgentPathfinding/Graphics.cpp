#include "Graphics.h"

#include <common/shader.hpp>
#include <iostream>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/detail/func_trigonometric.hpp>
#include <GLFW/glfw3.h>
#include <GL/glew.h>

ImVec2 Graphics::displaySize;
float Graphics::aspectRatio;

void Graphics::SetShaderUniforms(glm::vec3 inPosition, glm::vec3 inScale, glm::vec3 inColor)
{
	glm::mat4 model = glm::translate(mat4(1.f), inPosition) * glm::scale(mat4(1.f), inScale);
	glUniformMatrix4fv(mvpId, 1, GL_FALSE, &camera->getMVP()[0][0]);
	glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]);
	glUniform3f(colorId, inColor.x, inColor.y, inColor.z);
}

void Graphics::UpdateDisplaySize(int displayWidth, int displayHeight)
{
	displaySize.x = displayWidth;
	displaySize.y = displayHeight;
	aspectRatio = displaySize.x / displaySize.y;
}

Graphics::Graphics()
{
	isBatchActive = false;
	camera = new Camera();
}

Graphics::~Graphics()
{
}

bool Graphics::initGraphics()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n"); getchar();
		return false;
	}

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1600, 900, "Multi-agent Pathfinding", nullptr, nullptr);

	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar(); glfwTerminate(); 
		return false;
	}
	glfwMakeContextCurrent(window);


	// set window limits
	glfwSetWindowSizeLimits(window, GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE, 1600);
	//glfwSetWindowAspectRatio(window, 16, 9);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return false;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Set clear color
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);

	// Create the vertex array
	glGenVertexArrays(1, &vertexarray);
	glBindVertexArray(vertexarray);

	// Create and compile our GLSL program from the shaders
	singleColorShaderId = LoadShaders("SimpleColor.vertexshader", "SimpleColor.fragmentshader");

	// Get a handle for our "MVP" uniform
	mvpId = glGetUniformLocation(singleColorShaderId, "MVP");

	// Get a handle for our "Model" uniform
	modelId = glGetUniformLocation(singleColorShaderId, "Model");

	// Get a handle for our "Color" uniform
	colorId = glGetUniformLocation(singleColorShaderId, "Color");

	// Create quad vertex buffer object (VBO)
	const GLfloat quadVertexBufferData[] = {
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f,  -0.5f, 0.0f,
	};

	glGenBuffers(1, &squareVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, squareVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertexBufferData), quadVertexBufferData, GL_STATIC_DRAW);

	// Create a circle vertex buffer object (VBO)
	GLfloat circleVertexBufferData[48];
	double piPortion = 2 * 3.142 / static_cast<float>(16);
	for (int i = 0; i < 16; i++)
	{
		int base = i * 3;
		circleVertexBufferData[base] = 0.5f * glm::cos(piPortion * i); // x 
		circleVertexBufferData[base + 1] = 0.5f * glm::sin(piPortion * i); // y
		circleVertexBufferData[base + 2] = 0; // z
		printf("x: %f y: %f, z: %f\n", circleVertexBufferData[base], circleVertexBufferData[base + 1], circleVertexBufferData[base + 2]);
	}

	glGenBuffers(1, &circleVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, circleVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertexBufferData), circleVertexBufferData, GL_STATIC_DRAW);

	// Create a vertex buffer object for drawing a line (with points)
	glGenBuffers(1, &lineVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);

	// Create a texture buffer object (TBO)
	glGenTextures(1, &texturebuffer);
	glBindTexture(GL_TEXTURE_2D, texturebuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 600, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Create render buffer object (RBO)
	renderbuffer;
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 600, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	// Create frame buffer object (FBO)
	framebuffer;
	glGenFramebuffers(1, &framebuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

								   // Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texturebuffer, 0);

	// attach the renderbuffer to depth attachment point
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,	// 1. fbo target: GL_FRAMEBUFFER
		GL_DEPTH_ATTACHMENT,					// 2. attachment point
		GL_RENDERBUFFER,						// 3. rbo target: GL_RENDERBUFFER
		renderbuffer);							// 4. rbo ID

												// Check if the frame buffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		return false;
	}
}

void Graphics::CleanUp()
{
	glDeleteProgram(singleColorShaderId);

	// Cleanup vertex buffer VBO
	glDeleteBuffers(1, &squareVertexBuffer);
	glDeleteVertexArrays(1, &vertexarray);

	// Cleanup texture buffer TBO
	glDeleteBuffers(1, &texturebuffer);
	glDeleteBuffers(1, &renderbuffer);
	glDeleteFramebuffers(1, &framebuffer);
}

void Graphics::DrawTexture()
{
	// Draw the texture to the main window
	//ImTextureID tex_id = (ImTextureID) texturebuffer;
	//ImDrawList* draw_list = ImGui::GetWindowDrawList();
	//draw_list->PushTextureID(tex_id);
	//draw_list->AddImage(tex_id, ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y));
	//draw_list->PopTextureID();
	////ImGui::GetWindowDrawList()->PopTextureID();
}

//////////////////////////////////////////////////////////////////////////
// Draw Line
//////////////////////////////////////////////////////////////////////////

void Graphics::LineBatchBegin()
{
	// Use our shader
	glUseProgram(singleColorShaderId);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);

	glm::mat4 model = mat4(1.f);
	glUniformMatrix4fv(mvpId, 1, GL_FALSE, &camera->getMVP()[0][0]);
	glUniformMatrix4fv(modelId, 1, GL_FALSE, &model[0][0]);
}

void Graphics::LineBatchEnd()
{
	glDisableVertexAttribArray(0);
}

void Graphics::DrawLine(const std::vector<glm::vec3> points, glm::vec3 inColor, float lineWidth)
{
	if (points.empty()) return;

	glLineWidth(lineWidth);

	const int numPoints = points.size();

	GLfloat pointsVertexBufferData[300];

	for (int i = 0; i < points.size(); i++)
	{
		int newi = i * 2;
		pointsVertexBufferData[newi] = points[i].x;
		pointsVertexBufferData[newi + 1] = points[i].y;
		//pointsVertexBufferData[newi + 2] = points[i].z;
	}

	glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pointsVertexBufferData), pointsVertexBufferData, GL_STATIC_DRAW);

	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*) 0           // array buffer offset
	);

	glUniform3f(colorId, inColor.x, inColor.y, inColor.z);
	glDrawArrays(GL_LINE_STRIP, 0, numPoints); // 3 indices per point in the line
}

void Graphics::DrawBatch(glm::vec3 inPosition, glm::vec3 inColor, glm::vec3 inScale)
{
	if (!isBatchActive)
	{
		std::cerr << "ERROR: Shape Batch must be active to draw a circle" << std::endl;
		return;
	}

	SetShaderUniforms(inPosition, inScale, inColor);
	switch (shapeTypeForBatch)
	{
	case SHAPE_SQUARE:
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		break;
	case SHAPE_CIRCLE:
		glDrawArrays(GL_TRIANGLE_FAN, 0, 16);
		break;
	case SHAPE_NONE:
		break;
	default:
		break;
	}
}

void Graphics::ShapeBatchBegin(ShapeType inShape)
{
	isBatchActive = true;

	// Use our shader
	glUseProgram(singleColorShaderId);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);

	shapeTypeForBatch = inShape;

	GLfloat shapeVertexBuffer = squareVertexBuffer;
	switch (shapeTypeForBatch)
	{
	case SHAPE_SQUARE:
		shapeVertexBuffer = squareVertexBuffer;
		break;
	case SHAPE_CIRCLE:
		shapeVertexBuffer = circleVertexBuffer;
		break;
	}

	glBindBuffer(GL_ARRAY_BUFFER, shapeVertexBuffer);

	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*) 0            // array buffer offset
	);
}

void Graphics::ShapeBatchEnd()
{
	isBatchActive = false;
	glDisableVertexAttribArray(0);
	shapeTypeForBatch = SHAPE_NONE;
}


//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
//glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind FBO
//
//glBindTexture(GL_TEXTURE_2D, graphics.texturebuffer);
//glGenerateMipmap(GL_TEXTURE_2D);
//glBindTexture(GL_TEXTURE_2D, 0);