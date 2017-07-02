// https://gamedev.stackexchange.com/questions/140693/how-can-i-render-an-opegl-scene-into-an-imgui-window

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

#include <iostream>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include "Mesh.h"

static double g_Time = 0.0f;
static bool g_MousePressed[3] = { false, false, false };
static float g_MouseWheel = 0.0f;

int main(void)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 02 - Red triangle", nullptr, nullptr);
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


	ImGui_ImplGlfwGL3_Init(window, true);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	GLuint vertexarray;
	glGenVertexArrays(1, &vertexarray);
	glBindVertexArray(vertexarray);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("SimpleTransform.vertexshader", "SingleColor.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Get a handle for our "Model" uniform
	GLuint ModelID = glGetUniformLocation(programID, "Model");

	// Get a handle for our "Color" uniform
	GLuint ColorID = glGetUniformLocation(programID, "Color");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	//glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :

	float zoom = 50;
	glm::mat4 Projection = glm::ortho(-zoom, zoom, -zoom, zoom, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 0, 1), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);

	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f,  -0.5f, 0.0f,
	};

	// Create vertex buffer object (VBO)
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// Create a texture buffer object (TBO)
	GLuint texturebuffer;
	glGenTextures(1, &texturebuffer);
	glBindTexture(GL_TEXTURE_2D, texturebuffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 600, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	// Create render buffer object (RBO)
	GLuint renderbuffer;
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 600, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

	// Create frame buffer object (FBO)
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

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
		return -1;
	}

	ImVec4 clear_color = ImColor(114, 144, 154);

	int NUM_MESHES = 2000;
	int MAX_COLUMN_SIZE = 100;
	Mesh* meshes[2000];

	for (int i = 0; i < NUM_MESHES; i++)
	{
		int x = i % MAX_COLUMN_SIZE;
		int y = round(i / MAX_COLUMN_SIZE);
		meshes[i] = new Mesh(vec3(x * 2.5, y * 2.5, 0));//position.x = x * 1.1;
		//position.y = y * 1.1;
	}

	do
	{
		
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);


		//////////////////////////////////////////////////////////////////////////
		// Render a quad
		//////////////////////////////////////////////////////////////////////////

		//// Use our shader
		//glUseProgram(programID);

		//// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(0);
		//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		//glVertexAttribPointer(
		//	0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		//	3,                  // size
		//	GL_FLOAT,           // type
		//	GL_FALSE,           // normalized?
		//	0,                  // stride
		//	(void*) 0            // array buffer offset
		//);

		//// Draw the triangle strip!
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 3 indices starting at 0 -> 1 triangle

		//glDisableVertexAttribArray(0);

		//////////////////////////////////////////////////////////////////////////
		// Fill the texture buffer
		//////////////////////////////////////////////////////////////////////////

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//////////////////////////////////////////////////////////////////////////
		// Use our shader

		glViewport(0, 0, 600, 600);

		glUseProgram(programID);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*) 0            // array buffer offset
		);

		for (int i = 0; i < NUM_MESHES; i++)
		{
			meshes[i]->Update();
			//mat4 meshProjection = MVP * meshes[i]->model;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			glUniformMatrix4fv(ModelID, 1, GL_FALSE, &meshes[i]->model[0][0]);

			// Send our color to the shader
			glUniform3f(ColorID, 1, 1, 0);

			// Draw the triangle strip!
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 3 indices starting at 0 -> 1 triangle

			// Draw a line
			//glDrawArrays(GL_LINE_STRIP, 0, 4);
		}

		glDisableVertexAttribArray(0);
		//////////////////////////////////////////////////////////////////////////

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind FBO

		glBindTexture(GL_TEXTURE_2D, texturebuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		//////////////////////////////////////////////////////////////////////////
		// Send the texture buffer to an ImGUI window

		ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Window title Here");

		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();

		ImTextureID tex_id = (ImTextureID) texturebuffer;
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->PushTextureID(tex_id);
		draw_list->AddImage(tex_id, ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y));
		draw_list->PopTextureID();

		ImGui::Text("Hello, yo!");

		ImGui::End();

		////ImGui::GetWindowDrawList()->PopTextureID();

		//////////////////////////////////////////////////////////////////////////

		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window");
		ImGui::Text("Hello");
		ImGui::Text("Hello, title!");
		ImGui::End();


		// Rendering
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui::Render();
		glfwSwapBuffers(window);

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	// Cleanup vertex buffer VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &vertexarray);
	glDeleteProgram(programID);

	// Cleanup texture buffer TBO
	glDeleteBuffers(1, &texturebuffer);

	// Cleanup render buffer RBO
	glDeleteBuffers(1, &renderbuffer);

	// Cleanup frame buffer FBO
	glDeleteFramebuffers(1, &framebuffer);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}