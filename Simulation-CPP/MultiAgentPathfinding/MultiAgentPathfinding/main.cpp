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

//////////////////////////////////////////////////////////////////////////
// MOVE THIS INTO ITS OWN CLASS LATER
#include <scip/scip.h>
#include <scip/scipexception.h>
#include <scip/scipdefplugins.h>

// create variables
SCIP_VAR* xNum;

SCIP_RETCODE SetupProblem(SCIP* scip)
{
	// create empty problem
	SCIP_CALL_EXC(SCIPcreateProbBasic(scip, "string"));

	//SCIP_VAR* yNum;
	SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &xNum, "xNum", 0.0, SCIPinfinity(scip), 0.0, SCIP_VARTYPE_CONTINUOUS));
	//SCIP_CALL(SCIPcreateVarBasic(scip, &yNum, "yNum", 0.0, SCIPinfinity(scip), 0.0, SCIP_VARTYPE_CONTINUOUS));

	// add variables to problem
	SCIP_CALL_EXC(SCIPaddVar(scip, xNum));
	//SCIP_CALL(SCIPaddVar(scip, yNum));

	// setup expression for x
	double one = 1;
	SCIP_EXPR* X_Plus_Y;
	SCIP_CALL_EXC(SCIPexprCreate(SCIPblkmem(scip), &X_Plus_Y, SCIP_EXPR_VARIDX, 0));
	SCIP_CALL_EXC(SCIPexprCreateLinear(SCIPblkmem(scip), &X_Plus_Y, 1, &X_Plus_Y, &one, 2.0));

	// create linear constraint 
	const SCIP_Real MIN_X_VALUE = 3.0; // -SCIPinfinity(scip)
	const SCIP_Real MAX_X_VALUE = 5.0;
	SCIP_CONS* xMax;
	SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &xMax, "xMax", 0, nullptr, nullptr, MIN_X_VALUE, MAX_X_VALUE));
	SCIP_CALL_EXC(SCIPaddCoefLinear(scip, xMax, xNum, 1.0));

	// apply constraint
	SCIP_CALL_EXC(SCIPaddCons(scip, xMax));

	// release constraints
	// the problem has captured them and we do not require them any more
	SCIP_CALL_EXC(SCIPreleaseCons(scip, &xMax));

	return SCIP_OKAY;
}

void ScipTest()
{
	SCIP* scip;
	SCIP_CALL_EXC(SCIPcreate(&scip));
	SCIP_CALL_EXC(SCIPincludeDefaultPlugins(scip));

	SCIPinfoMessage(scip, nullptr, "\n");
	SCIPinfoMessage(scip, nullptr, "******************************\n");
	SCIPinfoMessage(scip, nullptr, "* Running Hello World solver *\n");
	SCIPinfoMessage(scip, nullptr, "******************************\n");
	SCIPinfoMessage(scip, nullptr, "\n");

	SCIP_CALL_EXC(SetupProblem(scip));


	SCIPinfoMessage(scip, nullptr, "Original problem:\n");
	SCIP_CALL_EXC(SCIPprintOrigProblem(scip, nullptr, "cip", FALSE));

	SCIPinfoMessage(scip, nullptr, "\n");
	SCIP_CALL_EXC(SCIPpresolve(scip));

	SCIPinfoMessage(scip, nullptr, "\nSolving...\n");
	SCIP_CALL_EXC(SCIPsolve(scip));

	SCIP_CALL_EXC(SCIPfreeTransform(scip));

	if (SCIPgetNSols(scip) > 0)
	{
		SCIPinfoMessage(scip, nullptr, "\nSolution:\n");
		SCIP_CALL_EXC(SCIPprintSol(scip, SCIPgetBestSol(scip), nullptr, FALSE));

		SCIP_SOL* Solution = SCIPgetBestSol(scip);
		//Solution->vals
		if (xNum)
		{
			double solution = SCIPgetSolVal(scip, Solution, xNum);
			std::cout << "xNum: " << solution << std::endl;
		}
	}

	// release variables
	SCIP_CALL_EXC(SCIPreleaseVar(scip, &xNum));

	SCIP_CALL_EXC(SCIPfree(&scip));

	
	
}

//////////////////////////////////////////////////////////////////////////


static double g_Time = 0.0f;
static bool g_MousePressed[3] = { false, false, false };
static float g_MouseWheel = 0.0f;


int main(void)
{
	ScipTest();

	Graphics graphics;
	if (!graphics.initGraphics())
		return -1;

	ImGui_ImplGlfwGL3_Init(graphics.window, true);

	// create the Simulation
	simulation = new Simulation();
	input = new Input();

	do
	{
		int display_w, display_h;
		glfwGetFramebufferSize(graphics.window, &display_w, &display_h);
		int minSize = min(display_w, display_h);
		int maxSize = max(display_w, display_h);

		// Rendering
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		// update the simulation and camera according to user input
		input->Update(0.16f, simulation, graphics.camera);
		float aspectRatio = static_cast<float>(display_w) / static_cast<float>(display_h);
		graphics.camera->update(0.16f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glViewport(0, 0, display_w, display_h);

		simulation->Render(&graphics);

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
		ImGui::Text("testing text");
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

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(graphics.window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(graphics.window) == 0);

	graphics.CleanUp();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}