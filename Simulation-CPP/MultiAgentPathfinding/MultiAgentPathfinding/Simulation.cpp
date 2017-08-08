#include "Simulation.h"
#include <iostream>
#include "Graphics.h"
#include "imgui.h"
#include "Options.h"
#include "Statistics.h"
#include <map>

int Simulation::timestep;

Simulation::Simulation()
{
	currentScenario = "warehouse.scenario";

	Stats::Reset();

	timestep = 0;
	aStar = new SpatialAStar(&environment.gridMap);
	coordinator = new AgentCoordinator(&environment.gridMap);

	scenario.LoadScenario("../scenarios/" + currentScenario, environment);
}


void Simulation::Reset()
{
	environment.Reset();
}

void Simulation::Step()
{
	for (Tile* tile : environment.gridMap.tiles)
	{
		if (tile->isWalkable)
			tile->color = vec3(1, 1, 1);
	}

	// allocate paths to agents which have no collisions
	coordinator->UpdateAgents(environment.agents);

	// we have resolved all conflicts, now move agents along their paths
	for (Agent* agent : environment.agents)
		agent->step();

	std::cout << "Updating timestep: " << timestep << std::endl;

	timestep += 1;
}

// #TODO  Move these options elsewhere

std::map<Agent*, bool> debugAgents;

void Simulation::Render(Graphics* graphics)
{
	if (Options::tickSimulation)
		Step();

	if (!Options::shouldRender) return;

	environment.Render(graphics);
	//if (Options::shouldShowPaths)
		//coordinator->DrawPotentialPaths(graphics, environment.agents);

	graphics->LineBatchBegin();
	for (Agent* agent : environment.agents)
	{
		if (Options::shouldShowPaths || debugAgents[agent])
			agent->drawPaths(graphics);
	}
	graphics->LineBatchEnd();

	// draw goals
	graphics->ShapeBatchBegin(SHAPE_SQUARE);
	for (Agent* agent : environment.agents)
		if (agent->goal)
			graphics->DrawBatch(glm::ivec3(agent->goal->x, agent->goal->y, 0), agent->color, glm::vec3(0.5f));
	graphics->ShapeBatchEnd();
	
}

void Simulation::BuildOptions()
{
	if (ImGui::Button("Reset scenario"))
	{
		scenario.LoadScenario("../scenarios/" + currentScenario, environment);
		coordinator = new AgentCoordinator(&environment.gridMap);
	}

	ImGui::Checkbox("Tick", &Options::tickSimulation);
	ImGui::Checkbox("Render", &Options::shouldRender);
	ImGui::Checkbox("Show paths", &Options::shouldShowPaths);

	for (Agent* agent : environment.agents)
	{
		char agentName[50];
		sprintf(agentName, "agent %d : %d paths (%d)", agent->getAgentId(), agent->allPaths.size(), agent->currentPath.size());
		ImGui::Checkbox(agentName, &debugAgents[agent]);
	}
}

void Simulation::LogInfo()
{
	ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("Timestep: %d", timestep);
	ImGui::Text("Number of agents: %d", environment.agents.size());
	ImGui::Text("Grid map dimensions: %d x %d", environment.gridMap.getWidth(), environment.gridMap.getHeight());
	ImGui::Text("Avg Search Time: %.9f", Stats::avgSearchTime);
	ImGui::Text("Avg Coordinator Time: %.9f", Stats::avgCoordinatorTime);
	ImGui::Text("Avg Mip Time: %.9f", Stats::avgMipTime);
}

void Simulation::BuildMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			ImGui::MenuItem("(dummy menu)", nullptr, false, false);
			ImGui::EndMenu();
		}
	}
	ImGui::EndMenuBar();
}

void Simulation::SelectTile(int mouseX, int mouseY)
{
	std::cout << "Selected Tile at " << mouseX << ", " << mouseY << std::endl;
}