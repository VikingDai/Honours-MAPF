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

class GridMap;
class AStar;
class Tile;

struct CollisionData
{
	Agent* agent;
	Tile* tile;
	std::deque<Tile*> path;
	int timestep;

	CollisionData(Agent* agent, Tile* tile, int timestep) :
		tile(tile), agent(agent), timestep(timestep)
	{
	}

	friend bool operator==(const CollisionData& otherdata, const CollisionData& data)
	{
		return	data.agent == otherdata.agent &&
			data.tile == otherdata.tile &&
			data.timestep == otherdata.timestep;
	}

	friend bool operator<(const CollisionData& lhs, const CollisionData& rhs)
	{
		return lhs.agent->getAgentId() < rhs.agent->getAgentId();
	}
};

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

	using AgentToPathsMap = std::map<Agent*, std::vector<AStar::Path*>>;
	AgentToPathsMap agentToPathsMap;

	std::set<CollisionData> collisions;

	std::vector<std::set<AStar::Path*>> CheckCollisions();

	void AddPath(Agent* agent, std::deque<Tile*>& path);

	void PopTimestep()
	{
		if (!tileToPathMapAtTimestep.empty()) 
			tileToPathMapAtTimestep.pop_front();
	}


	void BuildTilePathsMap();

	SCIP_RETCODE SetupProblem(SCIP* scip);
	void ScipSolve();

	AStar* aStar;
	GridMap* map;
	//ReservationTable table;

	std::vector<SCIP_VAR*> allVariables;

public:
	AgentCoordinator(GridMap* map);

	void FindAdjustedPath(AStar::Path& path);

	void UpdateAgents(std::vector<Agent*> agents);

	void DrawPotentialPaths(Graphics* graphics);
};

