#include "Input.h"
#include <glm/glm.hpp>
#include <imgui.h>
#include <iostream>

const int KEY_SPACE_BAR = 32;

void Input::Update(float deltaTime, Simulation* simulation)
{
	if (ImGui::IsKeyPressed(KEY_SPACE_BAR))
	{
		StepSimulation(simulation);
	}
	//std::cout << "Mouse clicked: " << ImGui::IsMouseClicked(0) << std::endl;
}

void Input::StepSimulation(Simulation* simulation)
{
	simulation->Step();
}
