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
private:
	TemporalAStar* aStar;
	GridMap* map;
	PathAssigner* pathAssigner;

public:
	AgentCoordinator(GridMap* map);
	
private:
	bool isRunning;
	Timer coordinatorTimer;
	Timer generatePathTimer;

private:
	void PrintPath(Agent* agent, TemporalAStar::Path& path);

private:
	using PathCollisions = std::vector<std::set<TemporalAStar::Path*>>;
	using TileToPathMap = std::map<Tile*, std::vector<AgentPath>>;
	
	/** (tile => time => num collisions) */
	using TileCollision = std::vector<std::pair<Tile*, int>>;

private:
	std::set<Agent*> agentsRequiringPath;
	std::map<Agent*, TileCollision> agentCollisionMap;
	TemporalAStar::TileCosts collisionCosts;
	std::deque<TileToPathMap> tileToPathMapAtTimestep;
	std::map<Tile*, std::map<int, std::vector<AgentPath>>> collisionTable;

private:
	bool Init(std::vector<Agent*>& agents);

	/** Generates a new path for an agent */
	void GeneratePath(
		Agent* agent, 
		std::map<Agent*, TileCollision> agentCollisionMap, 
		TemporalAStar::TileCosts& collisionCosts,
		bool firstRun);

	/** Check if any paths are in collision AND maps agents to tile collisions */
	std::vector<std::set<TemporalAStar::Path*>> CheckCollisions(std::vector<Agent*>& agents);

	/** */
	void BuildCollisionTable(std::vector<Agent*>& agents);

	/** Updates the collision table and stores any path collisions */
	void UpdateCollisions(TemporalAStar::Path& path);

private:
	TemporalAStar::Path* newestPath;

public:
	void Reset();

	/** Gives each agent a conflict-free path */
	void UpdateAgents(std::vector<Agent*>& agents);

	/** Generate paths and attempt to resolve conflicts for one iteration */
	bool Step(std::vector<Agent*>& agents);
};

