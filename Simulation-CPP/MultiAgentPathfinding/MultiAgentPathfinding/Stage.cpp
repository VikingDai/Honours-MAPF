#include "Stage.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/shader.hpp>

float Stage::zoom;
bool Stage::isLeftMouseDown;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		std::cout << "Pressed E" << std::endl;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		if (action == GLFW_PRESS)
			Stage::isLeftMouseDown = true;
		else if (action == GLFW_RELEASE)
			Stage::isLeftMouseDown = false;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	std::cout << yoffset << std::endl;
	Stage::zoom *= 1 + (yoffset * 0.05f);
}

void Stage::SetupInput(GLFWwindow* window)
{
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	//// Set keyboard input callback
	//glfwSetKeyCallback(window, key_callback);

	//// Set mouse button input callback
	//glfwSetMouseButtonCallback(window, mouse_button_callback);

	//// Set scroll wheel input callback
	//glfwSetScrollCallback(window, scroll_callback);
}


Stage::Stage()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return;
	}

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 1024, "Multi-agent pathfinding", nullptr, nullptr);
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return;
	}

	SetupInput(window);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("SimpleTransform.vertexshader", "SingleColor.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Get a handle for our "Color" uniform
	GLuint ColorID = glGetUniformLocation(programID, "Color");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	//glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f); // In world coordinates
	glm::mat4 Projection = glm::ortho(-zoom, zoom, -zoom, zoom, 0.0f, 5000.0f); // In world coordinates
																				// Camera matrix

	glm::mat4 View = glm::lookAt(
		cameraPos, // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);

	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f,	 0.5f,	0.5f,
		0.5f,	 0.5f,	0.5f,
		-0.5f,  -0.5f,	0.5f,
		0.5f,	-0.5f,	0.5f,
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
}


Stage::~Stage()
{
}

void Stage::Update(float dt)
{

}

void Stage::Render()
{
	
}
