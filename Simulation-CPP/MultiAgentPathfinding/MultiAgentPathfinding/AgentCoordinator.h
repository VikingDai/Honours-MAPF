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

class GridMap;
class AStar;
class Tile;

struct AgentPath
{
	Agent* agent;
	AStar::Path* path;

	AgentPath(Agent* agent, AStar::Path* path) : agent(agent), path(path)
	{

	}
};

class AgentCoordinator
{
	using TileToPathMap = std::map<Tile*, std::vector<AgentPath>>;
	std::deque<TileToPathMap> tileToPathMapAtTimestep;

	std::vector<std::set<AStar::Path*>> CheckCollisions(std::vector<Agent*>& agents);

	void AddPath(Agent* agent, std::deque<Tile*>& path);

	void PopTimestep()
	{
		//if (!tileToPathMapAtTimestep.empty())
		//	tileToPathMapAtTimestep.pop_front();
	}

	void BuildTable(std::vector<Agent*>& agents);

	void PrintAllPaths(std::vector<Agent*>& agents);

	// SCIP logics
	SCIP_RETCODE SetupProblem(SCIP* scip, std::vector<Agent*>& agents);
	void ResolveConflicts(std::vector<Agent*>& agents);

	std::vector<SCIP_VAR*> allVariables;
	std::map<SCIP_VAR*, Agent*> varToAgentMap;
	std::map<SCIP_VAR*, AStar::Path*> varToPathMap;
	std::map<SCIP_VAR*, char*> varNames;



public:
	AStar* aStar;
	GridMap* map;

public:
	AgentCoordinator(GridMap* map);

	void UpdateAgents(std::vector<Agent*>& agents);

	void FindAdjustedPath(AStar::Path& path);

	void DrawPotentialPaths(Graphics* graphics, std::vector<Agent*> agents);
};

