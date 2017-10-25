#pragma once
#include <vector>
#include <map>
#include <queue>
#include <deque>
#include <set>
#include <unordered_set>
#include "Agent.h";
#include "TemporalAStar.h"

#include "Timer.h"
#include "PathAssigner.h"

#include "MAPF.h"

#include <SFML/Graphics/RenderWindow.hpp>

class GridMap;
class Tile;

class AgentCoordinator
{
private:
	AStar aStar;
	GridMap* gridMap;
	PathAssigner pathAssigner;

	std::vector<MAPF::AgentPathRef*> usedPathRefs;

public:
	AgentCoordinator(GridMap* map);
	
	Timer timerCollisionDetection;
	Timer timerPathAssignment;
	Timer timerCoordinator;
	Timer timerPathGeneration;

private:
	int iteration;
	bool isRunning;

	MAPF::PathCollisions crossCollisionSet;
public:

	void Solve(std::vector<Agent*>& agents);

	void AssignShortestPath(std::vector<Agent*>& agents);

	int GetLongestPathLength(std::vector<Agent*>& agents);
	Tile* GetTileAtTimestep(MAPF::Path& path, int timestep);

	
	std::vector<MAPF::PathCollision> CheckForCollisions(std::vector<Agent*>& agents);

	void CreateEdgePenalties(CollisionPenalties& penalties, MAPF::AgentPathRef* pathRef);

	void GeneratePathsFromCollision(const MAPF::PathCollision collision);
	void GeneratePath(Agent* agent);
};