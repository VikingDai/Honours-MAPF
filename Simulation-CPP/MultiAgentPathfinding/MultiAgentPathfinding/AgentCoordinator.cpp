#include "AgentCoordinator.h"

#include "Agent.h"
#include "GridMap.h"
#include "AStar.h"
#include "Tile.h"
#include "Simulation.h"

//////////////////////////////////////////////////////////////////////////
// Reservation table
//////////////////////////////////////////////////////////////////////////

void ReservationTable::AddPath(Agent* agent, std::deque<Tile*> path)
{
	std::cout << "Added path" << std::endl;
	int newSize = path.size();
	if (agentsOnMap.size() < newSize)
		agentsOnMap.resize(newSize);

	for (int timestep = 0; timestep < path.size(); timestep++)
	{
		TileAgentsMap& tileAgentsMap = agentsOnMap[timestep];
		Tile* currentTile = path[timestep];
		tileAgentsMap[currentTile].push_back(agent);
	}
}

//////////////////////////////////////////////////////////////////////////
// Agent coordinator
//////////////////////////////////////////////////////////////////////////

AgentCoordinator::AgentCoordinator(GridMap* inMap)
{
	map = inMap;
	aStar = new AStar(inMap);
}

void AgentCoordinator::UpdateAgents(std::vector<Agent*> agents)
{
	table.PopTimestep();

	// update the reservation table with any new paths
	for (Agent* agent : agents)
	{
		if (agent->hasReachedGoal)
		{
			Tile* currentTile = map->getTileAt(agent->x, agent->y);
			Tile* goalTile = map->randomWalkableTile();
			agent->setPath(aStar->findPath(currentTile, goalTile));
			table.AddPath(agent, agent->getPath());
		}
	}

	for (int i = 0; i < table.agentsOnMap.size(); i++)
	{
		ReservationTable::TileAgentsMap agentMap = table.agentsOnMap[i];
		ReservationTable::TileAgentsMap::iterator it;
		for (it = agentMap.begin(); it != agentMap.end(); it++)
		{
			Tile* tile = it->first;
			//tile->color = vec3(1, 1, 0);

			std::vector<Agent*> agents = it->second;
			if (agents.size() > 1) // more than two agents are on this tile: there is a collision
				tile->color = vec3(1.f / (float) (i + 1), 0, 1.f / (float) (i + 1));

			//std::cout << "\t" <<  agents.size() << " agent(s) at: " << *tile << std::endl;
		}
	}
}

