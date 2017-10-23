#include "Simulation.h"
#include <iostream>
#include "imgui.h"
#include "Options.h"
#include "Statistics.h"
#include <map>
#include "AStar.h"
#include "TemporalBFS.h"
#include "TemporalAStar.h"

#include <string>
#include <iostream>
#include <filesystem>

#include <SFML/Graphics.hpp>
#include "Globals.h"
#include "Experiment.h"

#include <algorithm>

namespace fs = std::experimental::filesystem;

int Simulation::timestep;

MAPF::Path pathToDraw;
std::vector<MAPF::Path> pathsToDraw;

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

namespace ImGui2
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::Combo(label, currIndex, vector_getter, static_cast<void*>(&values), values.size());
	}

	bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values, int numToShow)
	{
		if (values.empty()) { return false; }
		return ImGui::ListBox(label, currIndex, vector_getter, static_cast<void*>(&values), values.size(), numToShow);
	}
}

Simulation::Simulation()
	: aStar(&environment.gridMap), temporalAStar(&environment.gridMap), bfs(&environment.gridMap)//, coordinator(&environment.gridMap)
{
	/** fill pools */
	AStarTileTime::FillPool(10000);

	seed = 0;
	srand(seed);

	iterations = 1;
	//currentScenario = "alternative";
	//currentScenario = "den520d-10";

	Stats::Reset();

	timestep = 0;

	// get files in scenario folder
	scenarioIndex = 0;
	std::string path = "../scenarios";
	for (fs::directory_entry entry : fs::directory_iterator(path))
		scenarioFiles.push_back(entry.path().string());


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
}


void Simulation::Update(float dt)
{
	for (Agent* agent : environment.agents)
	{
		agent->Update(dt);
	}
}

void Simulation::LoadScenario()
{
	Agent::ResetAgentCounter();

	if (Options::randomizeSeed) seed = rand();

	std::cout << "Set seed to " << seed << std::endl;
	srand(seed);

	scenario.LoadFromFile(scenarioFiles[scenarioIndex], environment);
	coordinator = new AgentCoordinator(&environment.gridMap);
	//environment.Reset();

	TemporalAStar* testAstar = new TemporalAStar(&environment.gridMap);
	Tile* start = environment.gridMap.GetTileAt(0, 0);
	Tile* goal = environment.gridMap.GetTileAt(8, 8);
}


void Simulation::Step()
{
	/*for (Tile* tile : environment.gridMap.tiles)
	{
		if (tile->isWalkable)
			tile->color = sf::Color(1, 1, 1);
	}*/

	// allocate paths to agents which have no collisions

	//coordinator->UpdateAgents(environment.agents);

	//if (coordinator->Step(environment.agents))
	//{
	//	// we have resolved all conflicts, now move agents along their paths
		for (Agent* agent : environment.agents)
			agent->Step();
	//}

	environment.gridMapRenderTexture.display();

	std::cout << "Updating timestep: " << timestep << std::endl;

	timestep += 1;
}

std::map<Agent*, bool> debugAgents;

void Simulation::Render(sf::RenderWindow& window)
{
	if (Options::tickSimulation)
		Step();

	if (!Options::shouldRender)
		return;

	for (Tile* tile : Tile::dirtyTiles)
		environment.DrawGridMapTile(tile);

	Tile::dirtyTiles.clear();

	/** Draw the grid map */
	environment.Render(window);

	/** Draw the agent */
	for (Agent* agent : environment.agents)
	{
		agent->DrawGoal(window);

		agent->DrawAgent(window);

		if (Options::shouldShowPaths || debugAgents[agent])
			agent->DrawPotentialPaths(window);

		if (Options::shouldShowLineToGoal)
			agent->DrawLineToGoal(window);

		if (Options::showPath)
			agent->DrawPath(window);
	}

	//if (Options::showCollisionCosts)
		//coordinator->RenderCollisionCosts(window);
}

void Simulation::BuildOptions()
{
	/** Make listbox to select a scenario */
	ImGui2::Combo("Select Scenario", &scenarioIndex, scenarioFiles);
	ImGui::InputInt("Seed", &seed);
	ImGui::Checkbox("RandomSeed", &Options::randomizeSeed);

	ImGui::InputInt("Iterations", &iterations);

	/** Load scenario button */
	if (ImGui::Button("Load scenario")) 
		LoadScenario();

	/** Run experiment button */
	if (ImGui::Button("Run experiment"))
	{
		Experiment experiment;
		experiment.RunExperiment("8x8", environment);
	}

	/** Add a solve button */
	if (ImGui::Button("Solve")) 
		coordinator->UpdateAgents2(environment.agents);
		//coordinator->UpdateAgents(environment.agents);

	ImGui::Checkbox("Tick", &Options::tickSimulation);
	ImGui::Checkbox("Render", &Options::shouldRender);
	ImGui::Checkbox("Show main path", &Options::showPath);
	ImGui::Checkbox("Show generated paths", &Options::shouldShowPaths);
	ImGui::Checkbox("Show line to goal", &Options::shouldShowLineToGoal);
	ImGui::Checkbox("Show collision costs", &Options::showCollisionCosts);

	for (Agent* agent : environment.agents)
	{
		char agentName[50];
		sprintf(agentName, "agent %d : %d paths", agent->GetAgentId(), agent->pathBank.size());
		ImGui::Checkbox(agentName, &debugAgents[agent]);
	}

	if (!simulationTimes.empty())
		ImGui::PlotHistogram("Histogram", &simulationTimes[0], simulationTimes.size(), 0, "Simulation Times", 0.0f, 100.f, ImVec2(0, 80));
}

void Simulation::LogInfo()
{
	ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("Timestep: %d", timestep);
	ImGui::Text("Number of agents: %d", environment.agents.size());
	ImGui::Text("Grid map dimensions: %d x %d", environment.gridMap.GetWidth(), environment.gridMap.GetHeight());
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

