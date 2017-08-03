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
#include <chrono>
#include <ctime>

static double g_TimeAcc;

int main()
{
	Graphics graphics;
	if (!graphics.initGraphics())
		return -1;

	ImGui_ImplGlfwGL3_Init(graphics.window, true);

	// create the Simulation
	simulation = new Simulation();
	input = new Input();

	std::chrono::time_point<std::chrono::system_clock> prevTime = std::chrono::system_clock::now();

	do
	{
		int display_w, display_h;
		glfwGetFramebufferSize(graphics.window, &display_w, &display_h);
		Graphics::displayWidth = display_w;
		Graphics::displayHeight = display_h;
		int minSize = min(display_w, display_h);
		int maxSize = max(display_w, display_h);

		// Rendering
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		// update the simulation and camera according to user input
		input->Update(0.16f, simulation, graphics.camera);
		float aspectRatio = static_cast<float>(display_w) / static_cast<float>(display_h);
		graphics.camera->update(0.16f);



		//if (g_TimeAcc > .033)
		{
			//g_TimeAcc -= .033;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glViewport(0, 0, display_w, display_h);

			simulation->Render(&graphics);
		}

		bool open = true;

		// Full screen window (invisible content, just for the menu bar)
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(display_w), static_cast<float>(display_h)), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("MenuBar", &open, ImVec2(0, 0), 0.f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
		simulation->BuildMenuBar();
		ImGui::End();

		// Options window (on the left)
		//ImGui::SetNextWindowSize(ImVec2((maxSize - minSize) / 2.f, static_cast<float>(display_h)), ImGuiSetCond_Always);
		ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiSetCond_Once);
		ImGui::Begin("Options", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Test: %f", g_TimeAcc);
		simulation->BuildOptions();
		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();

		// Draw the texture to the main window
		//ImTextureID tex_id = (ImTextureID) texturebuffer;
		//ImDrawList* draw_list = ImGui::GetWindowDrawList();
		//draw_list->PushTextureID(tex_id);
		//draw_list->AddImage(tex_id, ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y));
		//draw_list->PopTextureID();
		////ImGui::GetWindowDrawList()->PopTextureID();

		ImGui::End();

		ImVec2 padding(25.f, 75.f);

		// Info window (on the right)
		ImGui::SetNextWindowPos(ImVec2(display_w - (maxSize - minSize) / 2.f - padding.x, padding.y), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Example: Fixed Overlay", &open, ImVec2(0, 0), 0.2f, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
		simulation->LogInfo();
		ImGui::End();


		/*ImGui::SetNextWindowPos(ImVec2(10, 10));
		if (!ImGui::Begin("Example: Fixed Overlay", &open, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
		}
		ImGui::Text("Simple overlay\non the top-left side of the screen.");
		ImGui::Separator();
		ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
		ImGui::End();*/

		glViewport(0, 0, display_w, display_h);
		ImGui::Render();
		glfwSwapBuffers(graphics.window);

		// delta time
		std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
		std::chrono::duration<double> timeElapsed = currentTime - prevTime;
		g_TimeAcc += timeElapsed.count();
		prevTime = currentTime;

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(graphics.window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(graphics.window) == 0);

	graphics.CleanUp();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}