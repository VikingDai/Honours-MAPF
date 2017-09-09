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

#include <string>
#include <iostream>
#include <filesystem>

#include <SFML/Graphics.hpp>

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
{
	seed = 0;
	srand(seed);

	iterations = 1;
	//currentScenario = "alternative";
	//currentScenario = "den520d-10";

	Stats::Reset();

	timestep = 0;
	aStar = new TemporalAStar(&environment.gridMap);
	coordinator = new AgentCoordinator(&environment.gridMap);

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
}


void Simulation::LoadScenario()
{
	Agent::ResetAgentCounter();

	if (Options::randomizeSeed) seed = rand();

	std::cout << "Set seed to " << seed << std::endl;
	srand(seed);

	scenario.LoadScenario(scenarioFiles[scenarioIndex], environment);
	coordinator = new AgentCoordinator(&environment.gridMap);
	//environment.Reset();

	TemporalAStar* testAstar = new TemporalAStar(&environment.gridMap);
	Tile* start = environment.gridMap.getTileAt(0, 0);
	Tile* goal = environment.gridMap.getTileAt(8, 8);
}


void Simulation::Step()
{
	for (Tile* tile : environment.gridMap.tiles)
	{
		if (tile->isWalkable)
			tile->color = vec3(1, 1, 1);
	}

	// allocate paths to agents which have no collisions

	//coordinator->UpdateAgents(environment.agents);

	if (!coordinator->Step(environment.agents))
		return;

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

		if (Options::shouldShowLineToGoal)
			agent->drawLineToGoal(graphics);
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
		MAPF::Path& path = pathsToDraw[i];
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

void Simulation::Render2(sf::RenderWindow& window)
{
	environment.Render2(window);

	//for (int x = 0; x < 50; x++)
	//{
	//	for (int y = 0; y < 50; y++)
	//	{
	//		sf::RectangleShape rectangle(sf::Vector2f(1, 1));
	//		rectangle.setPosition(sf::Vector2f(x * 1.5, y * 1.5));
	//		window.draw(rectangle);
	//		//std::cout << x << "," << y << std::endl;
	//	}
	//}
}

void Simulation::BuildOptions()
{
	// Make listbox to select a scenario
	ImGui2::Combo("Select Scenario", &scenarioIndex, scenarioFiles);
	ImGui::InputInt("Seed", &seed);
	ImGui::Checkbox("RandomSeed", &Options::randomizeSeed);

	ImGui::InputInt("Iterations", &iterations);

	// Load scenario button
	if (ImGui::Button("Load scenario")) LoadScenario();

	if (ImGui::Button("Solve")) Step();

	ImGui::Checkbox("Tick", &Options::tickSimulation);
	ImGui::Checkbox("Render", &Options::shouldRender);
	ImGui::Checkbox("Show paths", &Options::shouldShowPaths);
	ImGui::Checkbox("Show line to goal", &Options::shouldShowLineToGoal);

	for (Agent* agent : environment.agents)
	{
		char agentName[50];
		sprintf(agentName, "agent %d : %d paths (%d)", agent->getAgentId(), agent->potentialPaths.size(), agent->getPath().size());
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