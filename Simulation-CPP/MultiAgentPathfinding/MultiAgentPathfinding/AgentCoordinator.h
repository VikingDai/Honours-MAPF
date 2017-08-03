#pragma once
#include <vector>
#include <map>
#include <queue>
#include <deque>
#include <set>
#include "Agent.h";
#include "AStar.h"
#include "Graphics.h"

// scip includes
#include <scip/scip.h>
#include <scip/scipexception.h>
#include <scip/scipdefplugins.h>
#include <scip/type_var.h>
#include <scip/struct_var.h>
#include "Timer.h"

class GridMap;
class Tile;

struct AgentPath
{
	Agent* agent;
	AStar::Path* path;

	AgentPath(Agent* agent, AStar::Path* path) : agent(agent), path(path) {}
};

class AgentCoordinator
{
	using PathCollisions = std::vector<std::set<AStar::Path*>>;

	Timer mipTimer;
	Timer coordinatorTimer;

	std::map<Tile*, std::map<int, std::vector<AgentPath>>> collisionTable;

	using TileToPathMap = std::map<Tile*, std::vector<AgentPath>>;
	std::deque<TileToPathMap> tileToPathMapAtTimestep;

	// (tile => time => num collisions)
	using TileCollision = std::set<std::pair<Tile*, int>>;

	// agent => (tile => time => num collisions)
	//std::map<Agent*, TileCollision> agentsInCollision;

	std::vector<std::set<AStar::Path*>> CheckCollisions(std::vector<Agent*>& agents, std::map<Agent*, TileCollision>& agentsInCollision);
	std::vector<std::pair<Tile*, int>> TilesInCollision(Agent* agent, AStar::Path& path);

	std::map<AStar::Path*, int> PathLengths;

	void PopTimestep()
	{
		//if (!tileToPathMapAtTimestep.empty())
		//	tileToPathMapAtTimestep.pop_front();
	}

	void BuildTable(std::vector<Agent*>& agents);

	void PrintAllPaths(std::vector<Agent*>& agents);
	void PrintPath(Agent* agent, AStar::Path& path);

	// SCIP functions
	std::vector<Agent*> ResolveConflicts(std::vector<Agent*>& agents, PathCollisions& collisions);
	SCIP_RETCODE SetupProblem(SCIP* scip, std::vector<Agent*>& agents, PathCollisions& collisions);

	// SCIP helper structures
	std::vector<SCIP_VAR*> allVariables;
	std::map<SCIP_VAR*, Agent*> varToAgentMap;
	std::map<SCIP_VAR*, AStar::Path*> varToPathMap;
	std::map<SCIP_VAR*, char*> varNames;

public:
	AStar* aStar;
	GridMap* map;

public:
	AgentCoordinator(GridMap* map);

	void Reset();

	void UpdateAgents(std::vector<Agent*>& agents);

	void GeneratePath(Agent* agent, bool useCollisions, std::map<Agent*, TileCollision> agentCollisionMap);

	void DrawPotentialPaths(Graphics* graphics, std::vector<Agent*> agents);
};

