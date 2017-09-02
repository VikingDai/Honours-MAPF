#include "Simulation.h"
#include <iostream>
#include "Graphics.h"
#include "imgui.h"
#include "Options.h"
#include "Statistics.h"
#include <map>
#include "AStar.h"
#include "TemporalBFS.h"
#include "TemporalAStar.h"

int Simulation::timestep;

TemporalAStar::Path pathToDraw;
std::vector<TemporalAStar::Path> pathsToDraw;

Simulation::Simulation()
{
	currentScenario = "alternative.scenario";
	//currentScenario = "den520d-10.scenario";

	Stats::Reset();

	timestep = 0;
	aStar = new TemporalAStar(&environment.gridMap);
	coordinator = new AgentCoordinator(&environment.gridMap);

	scenario.LoadScenario("../scenarios/" + currentScenario, environment);

	//////////////////////////////////////////////////////////////////////////
	// TEST SPATIAL A*

	/*TemporalAStar TemporalAStar(&environment.gridMap);
	Tile* start = environment.gridMap.getTileAt(2, 2);
	Tile* goal = environment.gridMap.getTileAt(4, 4);

	start->color = glm::vec3(1, 1, 0);
	goal->color = glm::vec3(1, 1, 0);*/

	/*for (int i = 0; i < 50; i++)
	{
		std::cout << "FINDING PATH " << i << std::endl;
		pathsToDraw.push_back(TemporalAStar.FindPath2(start, goal));
		std::cout << std::endl;
	}*/

	// END TEST SPATIAL A*
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// TEST BFS
	
	bfs = new TemporalBFS(&environment.gridMap);

	//TemporalBFS bfs(&environment.gridMap);
	//Tile* start = environment.gridMap.getTileAt(2, 2);
	//Tile* goal = environment.gridMap.getTileAt(2, 6);

	//start->color = glm::vec3(1, 1, 0);
	//goal->color = glm::vec3(1, 1, 0);

	////pathsToDraw = bfs.SearchToDepth(start, goal, 2);

	//Timer bfsTimer;
	//for (int i = 1; i < 8; i++)
	//{
	//	bfsTimer.Reset();
	//	bfsTimer.Begin();
	//	std::cout << "##############################" << std::endl;
	//	std::cout << "GENERATING PATHS OF DEPTH " << i << std::endl;
	//	std::cout << "Found " << bfs.SearchToDepth(start, goal, i).size() << " paths " << std::endl;
	//	bfsTimer.End();
	//	bfsTimer.PrintTimeElapsed("GENERATING TOOK");
	//}

	/*for (TemporalBFS::Path path : pathsToDraw)
	{
		for (Tile* tile : path)
			std::cout << *tile << " > ";
		std::cout << std::endl;
	}*/

	// END TEST BFS
	//////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////
	// TEST A*

	//AStar* testAstar = new AStar(&environment.gridMap);
	//Tile* start = environment.gridMap.getTileAt(0, 0);
	//Tile* goal = environment.gridMap.getTileAt(8, 8);

	//start->color = glm::vec3(1, 1, 0);
	//goal->color = glm::vec3(1, 1, 0);

	//AStar::Path& path = testAstar->FindPath(start, goal);
	/*while (!path.empty())
	{
		path.front()->color = glm::vec3(0, 0, 1);
		std::cout << *path.front() << std::endl;
		path.pop();
	}*/

	//pathsToDraw = testAstar->FindPaths(start, goal);
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

	//Tile* start = environment.gridMap.getTileAt(2, 1);
	//Tile* goal = environment.gridMap.getTileAt(0, 1);
	//pathToDraw = bfs->FindNextPath(start, goal);
	///*TemporalBFS::Path& path = bfs->FindNextPath(start, goal);
	//pathsToDraw.push_back(path);*/

	//std::cout << "Found path: " << std::endl;
	//// print the path
	//for (Tile* tile : pathToDraw)
	//	std::cout << *tile << " > " << std::endl;
	//std::cout << std::endl;

	//start->color = glm::vec3(1, 0, 0);
	//goal->color = glm::vec3(0, 1, 0);

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
	
	////////////////////////////////////////////////////////////////////////////// TESTING
	graphics->LineBatchBegin();
	float sep = .6;
	for (int i = 0; i < pathsToDraw.size(); i++)
	{
		TemporalBFS::Path& path = pathsToDraw[i];
		float pathSep = sep / pathsToDraw.size();
		std::vector<vec3> points;
		for (Tile* tile : path)
		{
			points.emplace_back(vec3(tile->x + pathSep * i - sep / 2, tile->y + pathSep * i - sep / 2, 0));
		}

		graphics->DrawLine(points, glm::vec3(0, 1, 1), 2.5f);
		points.clear();
	}
	graphics->LineBatchEnd();

	////////////////////////////////////////////////////////////////////////// TESTING 2
	graphics->LineBatchBegin();
	std::vector<vec3> points;
	for (Tile* tile : pathToDraw)
	{
		points.emplace_back(vec3(tile->x, tile->y, 0));
	}
	graphics->DrawLine(points, glm::vec3(0, 1, 1), 2.5f);
	points.clear();
	
	graphics->LineBatchEnd();
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