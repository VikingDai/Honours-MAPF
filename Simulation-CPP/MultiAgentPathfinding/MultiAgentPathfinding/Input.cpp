#include "Input.h"
#include <glm/glm.hpp>
#include <imgui.h>
#include <iostream>
#include "Simulation.h"
#include "camera.h"

const int KEY_SPACE_BAR = 32;

void Input::Update(float deltaTime, Simulation* simulation, Camera* camera)
{
	if (ImGui::IsKeyPressed(KEY_SPACE_BAR))
	{
		StepSimulation(simulation);
	}


	if (ImGui::IsMouseClicked(0))
	{
		camera->getMVP();
		camera->zoom;
		camera->position;
		simulation->SelectTile(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	}

	// Update camera
	if (ImGui::IsMouseDown(0))
	{
		camera->position.x += -1 * ImGui::GetIO().MouseDelta.x * deltaTime * camera->zoom / 100.f;
		camera->position.y += ImGui::GetIO().MouseDelta.y * deltaTime * camera->zoom / 100.f;
	}

	camera->zoom *= 1 - (0.075f * ImGui::GetIO().MouseWheel);
}

void Input::StepSimulation(Simulation* simulation)
{
	simulation->Step();
}
