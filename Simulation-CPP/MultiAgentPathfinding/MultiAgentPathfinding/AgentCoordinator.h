#pragma once
#include <vector>
#include <map>
#include <queue>
#include <deque>

class Agent;
class GridMap;
class AStar;
class Tile;

struct ReservationTable
{
	using TileAgentsMap = std::map<Tile*, std::vector<Agent*>>;
	std::deque<TileAgentsMap> agentsOnMap;

	void AddPath(Agent* agent, std::deque<Tile*> path);
	void PopTimestep() 
	{ 
		if (!agentsOnMap.empty()) agentsOnMap.pop_front(); 
	}
};

class AgentCoordinator
{
	AStar* aStar;
	GridMap* map;
	ReservationTable table;

public:
	AgentCoordinator(GridMap* map);

	void UpdateAgents(std::vector<Agent*> agents);
};

