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

#include "Simulation.h"
Simulation* simulation;

#include "Input.h"
Input* input;

#include "Camera.h"
#include "Graphics.h"
Camera* camera;

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
	window = glfwCreateWindow(1600, 900, "Tutorial 02 - Red triangle", nullptr, nullptr);

	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
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
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	ImGui_ImplGlfwGL3_Init(window, true);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	Graphics graphics;
	if (!graphics.initGraphics())
		return -1;

	// create the camera
	camera = new Camera();

	// create the Simulation
	simulation = new Simulation();
	input = new Input();

	ImVec4 clear_color = ImColor(114, 144, 154);

	int NUM_MESHES = 2000;
	int MAX_COLUMN_SIZE = 50;
	Mesh* meshes[2000];

	for (int i = 0; i < NUM_MESHES; i++)
	{
		int x = i % MAX_COLUMN_SIZE;
		int y = round(i / MAX_COLUMN_SIZE);
		meshes[i] = new Mesh(vec3(x * 2.5, -100 + y * 2.5, 0));//position.x = x * 1.1;
		//position.y = y * 1.1;
	}

	do
	{
		// Rendering
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);

		// update the simulation and camera according to user input
		input->Update(0.16f, simulation, camera);

		float aspectRatio = static_cast<float>(display_w) / static_cast<float>(display_h);
		//Projection = glm::ortho(-zoom, zoom, -zoom, zoom, 0.f, 100.f) * glm::scale(mat4(1.f), vec3(1.f / aspectRatio, 1.f, 1.f));
		//Model = scale(mat4(3.0f), vec3(1.f / aspectRatio, 1, 1));
		//MVP = Projection * View * Model;

		camera->update(0.16f);

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

		
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//////////////////////////////////////////////////////////////////////////
		// Use our shader
		int minSize = min(display_w, display_h);
		int maxSize = max(display_w, display_h);
		//glViewport((maxSize - minSize) / 2, 0, minSize, maxSize);
		glViewport(0, 0, display_w, display_h);

		glUseProgram(graphics.programID);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, graphics.vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*) 0            // array buffer offset
		);

		//for (int i = 0; i < NUM_MESHES; i++)
		//{
			//meshes[i]->Update();
		for (Tile* tile : simulation->environment.gridMap.tiles)
		{
			if (!tile) continue;
			
			glUniformMatrix4fv(graphics.MVPID, 1, GL_FALSE, &camera->getMVP()[0][0]);
			glUniformMatrix4fv(graphics.ModelID, 1, GL_FALSE, &tile->mesh->model[0][0]);

			// Send our color to the shader
			glUniform3f(graphics.ColorID, 1, 1, 0);

			// Draw the triangle strip!
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 3 indices starting at 0 -> 1 triangle

			// Draw a line
			//glDrawArrays(GL_LINE_STRIP, 0, 4);
		}

		glDisableVertexAttribArray(0);
		//////////////////////////////////////////////////////////////////////////

		//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind FBO

		glBindTexture(GL_TEXTURE_2D, graphics.texturebuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		//////////////////////////////////////////////////////////////////////////
		// Send the texture buffer to an ImGUI window

		bool open = true;

		ImGui::SetNextWindowSize(ImVec2((maxSize - minSize) / 2, display_h));
		//ImGui::SetNextWindowSize(ImVec2(300, 300));
		ImGui::Begin("Window title Here", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);

		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();

		// Draw the texture to the main window
		//ImTextureID tex_id = (ImTextureID) texturebuffer;
		//ImDrawList* draw_list = ImGui::GetWindowDrawList();
		//draw_list->PushTextureID(tex_id);
		//draw_list->AddImage(tex_id, ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y));
		//draw_list->PopTextureID();

		ImGui::Text("Helreally long text you know really longlo, yo!");

		ImGui::End();

		////ImGui::GetWindowDrawList()->PopTextureID();

		//////////////////////////////////////////////////////////////////////////
		ImGui::SetNextWindowSize(ImVec2((maxSize - minSize) / 2, display_h));
		ImGui::SetNextWindowPos(ImVec2(display_w - (maxSize - minSize) / 2, 0));
		//ImGui::Begin("Test");
		ImGui::Begin("Testing", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::Text("Hello");
		ImGui::Text("Hello, title!");
		ImGui::End();

		glViewport(0, 0, display_w, display_h);
		ImGui::Render();
		glfwSwapBuffers(window);

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	//// Cleanup vertex buffer VBO
	//glDeleteBuffers(1, &vertexbuffer);
	//glDeleteVertexArrays(1, &vertexarray);
	//glDeleteProgram(programID);

	//// Cleanup texture buffer TBO
	//glDeleteBuffers(1, &texturebuffer);

	//// Cleanup render buffer RBO
	//glDeleteBuffers(1, &renderbuffer);

	//// Cleanup frame buffer FBO
	//glDeleteFramebuffers(1, &framebuffer);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}