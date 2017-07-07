#include "Simulation.h"
#include <iostream>
#include "Graphics.h"
#include "imgui.h"


Simulation::Simulation()
{
	timestep = 0;
	aStar = new AStar(&environment.gridMap);
}


Simulation::~Simulation()
{
}

void Simulation::Step()
{
	for (Agent* agent : environment.agents)
	{
		agent->Step();
	}


	//Tile* start = environment.gridMap.getTileAt(1, 1);
	//Tile* goal = environment.gridMap.getTileAt(15, 19);
	//std::vector<Tile*> path = aStar->findPath(start, goal);

	//for (Tile* tile : path)
	//{
	//	tile->color = vec3(0, 1, 0);
	//	std::cout << tile->x << "," << tile->y << std::endl;
	//}

	//start->color = vec3(1, 0, 0);
	//goal->color = vec3(1, 0, 0);

	std::cout << "Updating timestep: " << timestep << std::endl;

	timestep += 1;
}

void Simulation::Render(Graphics* graphics)
{
	environment.Render(graphics);
}

void Simulation::LogInfo()
{
	ImGui::Text("Timestep: %d", timestep);
	ImGui::Text("Number of agents: %d", environment.agents.size());
	ImGui::Text("Grid map dimensions: %d x %d", environment.gridMap.getWidth(), environment.gridMap.getHeight());
}

void Simulation::BuildMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			ImGui::MenuItem("(dummy menu)", NULL, false, false);
			ImGui::EndMenu();
		}
	}
	ImGui::EndMenuBar();
}
