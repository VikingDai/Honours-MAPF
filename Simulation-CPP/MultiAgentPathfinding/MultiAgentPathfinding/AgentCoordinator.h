#pragma once
#include <vector>
#include <map>
#include <queue>
#include <deque>
#include <set>
#include "Agent.h";
#include "TemporalAStar.h"
#include "Graphics.h"

#include "Timer.h"
#include "PathAssigner.h"

class GridMap;
class Tile;

struct AgentPath
{
	Agent* agent;
	TemporalAStar::Path* path;

	AgentPath(Agent* agent, TemporalAStar::Path* path) : agent(agent), path(path) {}
};

class AgentCoordinator
{
public:
	AgentCoordinator(GridMap* map);

private:
	PathAssigner* pathAssigner;

	Timer coordinatorTimer;
	Timer generatePathTimer;

	using PathCollisions = std::vector<std::set<TemporalAStar::Path*>>;
	using TileToPathMap = std::map<Tile*, std::vector<AgentPath>>;
	
	// (tile => time => num collisions)
	using TileCollision = std::vector<std::pair<Tile*, int>>;

	// agent => (tile => time => num collisions)
	//std::map<Agent*, TileCollision> agentsInCollision;

	std::deque<TileToPathMap> tileToPathMapAtTimestep;
	std::map<Tile*, std::map<int, std::vector<AgentPath>>> collisionTable;

	/* Check if any paths are in collision AND maps agents to tile collisions */
	std::vector<std::set<TemporalAStar::Path*>> CheckCollisions(std::vector<Agent*>& agents, std::map<Agent*, TileCollision>& agentsInCollision);
	std::vector<std::pair<Tile*, int>> TilesInCollision(Agent* agent, TemporalAStar::Path& path);

	void BuildCollisionTable(std::vector<Agent*>& agents);

	void PrintAllPaths(std::vector<Agent*>& agents);
	void PrintPath(Agent* agent, TemporalAStar::Path& path);

private:
	/** Generates a new path for an agent */
	void GeneratePath(
		Agent* agent, 
		bool useCollisions, 
		std::map<Agent*, TileCollision> agentCollisionMap, 
		TemporalAStar::TileCosts& collisionCosts);


public:
	TemporalAStar* aStar;
	GridMap* map;

public:

	void Reset();

	/** Gives each agent a conflict-free path */
	void UpdateAgents(std::vector<Agent*>& agents);
};

