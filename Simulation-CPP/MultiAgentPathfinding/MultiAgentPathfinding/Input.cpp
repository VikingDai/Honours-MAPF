#include "Input.h"
#include <glm/glm.hpp>
#include <imgui.h>
#include <iostream>
#include "Simulation.h"

const int KEY_SPACE_BAR = 32;

void Input::Update(float deltaTime, Simulation* simulation)
{
	if (ImGui::IsKeyPressed(KEY_SPACE_BAR))
	{
		StepSimulation(simulation);
	}

	//if (ImGui::IsMouseClicked(0))
	//{
	//	camera->getMVP();
	//	camera->zoom;
	//	camera->position;
	//	
	//	ImVec2 mouseWorld = ImVec2(
	//		ImGui::GetMousePos().x - Graphics::GetDisplaySize().x * 0.5f,
	//		ImGui::GetMousePos().y - Graphics::GetDisplaySize().y * 0.5f);

	//	std::cout << ImGui::GetMousePos().x << " / " << Graphics::GetDisplaySize().x << std::endl;

	//	std::cout << mouseWorld.x << " , " << mouseWorld.y << std::endl;

	//	simulation->SelectTile(
	//		(ImGui::GetMousePos().x - Graphics::GetDisplaySize().x * 0.5f + camera->position.x) / camera->zoom,
	//		(ImGui::GetMousePos().y - Graphics::GetDisplaySize().y * 0.5f + camera->position.y) / camera->zoom);
	//}

	//// Update camera
	//if (ImGui::IsMouseDown(0))
	//{
	//	camera->position.x += -1 * ImGui::GetIO().MouseDelta.x * deltaTime * camera->zoom / 100.f;
	//	camera->position.y += ImGui::GetIO().MouseDelta.y * deltaTime * camera->zoom / 100.f;
	//}

	//camera->zoom *= 1 - (0.075f * ImGui::GetIO().MouseWheel);
}

void Input::StepSimulation(Simulation* simulation)
{
	simulation->Step();
}
