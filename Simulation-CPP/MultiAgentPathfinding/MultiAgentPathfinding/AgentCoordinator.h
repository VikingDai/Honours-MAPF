#pragma once
#include <vector>
#include <map>
#include <queue>
#include <deque>
#include <set>
#include "Agent.h";
#include "SpatialAStar.h"
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
	SpatialAStar::Path* path;

	AgentPath(Agent* agent, SpatialAStar::Path* path) : agent(agent), path(path) {}
};

class AgentCoordinator
{
	Timer mipTimer;
	Timer coordinatorTimer;

	using PathCollisions = std::vector<std::set<SpatialAStar::Path*>>;
	using TileToPathMap = std::map<Tile*, std::vector<AgentPath>>;
	
	// (tile => time => num collisions)
	using TileCollision = std::set<std::pair<Tile*, int>>;

	// agent => (tile => time => num collisions)
	//std::map<Agent*, TileCollision> agentsInCollision;

	std::deque<TileToPathMap> tileToPathMapAtTimestep;
	std::map<Tile*, std::map<int, std::vector<AgentPath>>> collisionTable;

	std::vector<std::set<SpatialAStar::Path*>> CheckCollisions(std::vector<Agent*>& agents, std::map<Agent*, TileCollision>& agentsInCollision);
	std::vector<std::pair<Tile*, int>> TilesInCollision(Agent* agent, SpatialAStar::Path& path);

	std::map<SpatialAStar::Path*, int> PathLengths;

	void PopTimestep()
	{
		//if (!tileToPathMapAtTimestep.empty())
		//	tileToPathMapAtTimestep.pop_front();
	}

	void BuildTable(std::vector<Agent*>& agents);

	void PrintAllPaths(std::vector<Agent*>& agents);
	void PrintPath(Agent* agent, SpatialAStar::Path& path);

	// SCIP functions
	std::vector<Agent*> ResolveConflicts(std::vector<Agent*>& agents, PathCollisions& collisions);
	SCIP_RETCODE SetupProblem(SCIP* scip, std::vector<Agent*>& agents, PathCollisions& collisions);

	// SCIP helper structures
	std::vector<SCIP_VAR*> allVariables;
	std::map<SCIP_VAR*, Agent*> varToAgentMap;
	std::map<SCIP_VAR*, SpatialAStar::Path*> varToPathMap;
	std::map<SCIP_VAR*, char*> varNames;

public:
	SpatialAStar* aStar;
	GridMap* map;

public:
	AgentCoordinator(GridMap* map);

	void Reset();

	void UpdateAgents(std::vector<Agent*>& agents);

	void GeneratePath(Agent* agent, bool useCollisions, std::map<Agent*, TileCollision> agentCollisionMap);

	void DrawPotentialPaths(Graphics* graphics, std::vector<Agent*> agents);
};

