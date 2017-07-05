// https://gamedev.stackexchange.com/questions/140693/how-can-i-render-an-opegl-scene-into-an-imgui-window

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

#include <iostream>

// Include GLFW
#include <GLFW/glfw3.h>
//GLFWwindow* window;

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

#include "Graphics.h"

static double g_Time = 0.0f;
static bool g_MousePressed[3] = { false, false, false };
static float g_MouseWheel = 0.0f;


int main(void)
{
	Graphics graphics;
	if (!graphics.initGraphics())
		return -1;

	ImGui_ImplGlfwGL3_Init(graphics.window, true);

	// create the Simulation
	simulation = new Simulation();
	input = new Input();

	do
	{
		// Rendering
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		int display_w, display_h;
		glfwGetFramebufferSize(graphics.window, &display_w, &display_h);

		// update the simulation and camera according to user input
		input->Update(0.16f, simulation, graphics.camera);
		float aspectRatio = static_cast<float>(display_w) / static_cast<float>(display_h);
		graphics.camera->update(0.16f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		int minSize = min(display_w, display_h);
		int maxSize = max(display_w, display_h);
		glViewport(0, 0, display_w, display_h);

		simulation->Render(&graphics);

		////////////////////////////////////////////////////////////////////////////
		//// Use our shader
		//glUseProgram(graphics.programID);

		//// 1rst attribute buffer : vertices
		//glEnableVertexAttribArray(0);
		//glBindBuffer(GL_ARRAY_BUFFER, graphics.vertexbuffer);
		//glVertexAttribPointer(
		//	0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		//	3,                  // size
		//	GL_FLOAT,           // type
		//	GL_FALSE,           // normalized?
		//	0,                  // stride
		//	(void*) 0            // array buffer offset
		//);

		////for (int i = 0; i < NUM_MESHES; i++)
		////{
		//	//meshes[i]->Update();
		//for (Tile* tile : simulation->environment.gridMap.tiles)
		//{
		//	if (!tile) continue;
		//	
		//	glUniformMatrix4fv(graphics.MVPID, 1, GL_FALSE, &camera->getMVP()[0][0]);
		//	glUniformMatrix4fv(graphics.ModelID, 1, GL_FALSE, &tile->mesh->model[0][0]);

		//	// Send our color to the shader
		//	glUniform3f(graphics.ColorID, 1, 1, 0);

		//	// Draw the triangle strip!
		//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 3 indices starting at 0 -> 1 triangle

		//	// Draw a line
		//	//glDrawArrays(GL_LINE_STRIP, 0, 4);
		//}

		//glDisableVertexAttribArray(0);
		////////////////////////////////////////////////////////////////////////////

		//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind FBO

		glBindTexture(GL_TEXTURE_2D, graphics.texturebuffer);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		//////////////////////////////////////////////////////////////////////////
		// Send the texture buffer to an ImGUI window

		bool open = true;
		ImGui::SetNextWindowSize(ImVec2((maxSize - minSize) / 2.f, static_cast<float>(display_h)), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Options", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);

		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();

		// Draw the texture to the main window
		//ImTextureID tex_id = (ImTextureID) texturebuffer;
		//ImDrawList* draw_list = ImGui::GetWindowDrawList();
		//draw_list->PushTextureID(tex_id);
		//draw_list->AddImage(tex_id, ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y));
		//draw_list->PopTextureID();

		ImGui::Text("Sample text");

		ImGui::End();
		////ImGui::GetWindowDrawList()->PopTextureID();

		//////////////////////////////////////////////////////////////////////////
		ImGui::SetNextWindowSize(ImVec2((maxSize - minSize) / 2, display_h), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(display_w - (maxSize - minSize) / 2.f, 0), ImGuiSetCond_FirstUseEver);
		//ImGui::Begin("Test");
		ImGui::Begin("Testing", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::Text("Hello");
		ImGui::Text("Hello, title!");
		ImGui::End();

		glViewport(0, 0, display_w, display_h);
		ImGui::Render();
		glfwSwapBuffers(graphics.window);

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(graphics.window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(graphics.window) == 0);

	graphics.CleanUp();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}