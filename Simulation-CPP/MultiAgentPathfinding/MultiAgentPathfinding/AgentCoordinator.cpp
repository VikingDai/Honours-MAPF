#include "AgentCoordinator.h"

#include "Agent.h"
#include "GridMap.h"
#include "AStar.h"
#include "Tile.h"
#include "Simulation.h"

//////////////////////////////////////////////////////////////////////////
// Reservation table
//////////////////////////////////////////////////////////////////////////

void AgentCoordinator::AddPath(Agent* agent, AStar::Path& path)
{
	
	int newSize = path.size();

	std::cout << "Added path of length " << newSize << std::endl;

	if (tileToPathMapAtTimestep.size() < newSize)
		tileToPathMapAtTimestep.resize(newSize);

	// associate the path to the agent
	std::vector<AStar::Path>& paths = agentToPathsMap[agent];
	paths.push_back(path);

	for (int timestep = 0; timestep < path.size(); timestep++)
	{
		// associate each tile in the path to a timestep and pointer to the path
		Tile* currentTile = path[timestep];
		TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
		std::vector<AStar::Path*>& pathsUsingTile = tilePathMap[currentTile];
		pathsUsingTile.push_back(&path);

		//// check for collisions
		//if (agentsOnTile.size() > 1)
		//{
		//	collisions.insert(CollisionData(agent, currentTile, timestep + Simulation::timestep));
		//}
		//else if (timestep != 0) // check if we moved into a position where an agent was last timestep
		//{
		//	TileAgentsMap& previousTileAgentMap = agentsOnMap[timestep - 1];
		//	if (!previousTileAgentMap[currentTile].empty())
		//	{
		//		for (Agent* otherAgent : previousTileAgentMap[currentTile])
		//		{
		//			if (timestep < otherAgent->getPath().size())
		//			{
		//				//collisions.emplace_back(agent, currentTile, timestep + Simulation::timestep);
		//				//std::cout << "This is checking" << std::endl;
		//				Tile* otherAgentCurrent = otherAgent->getPath()[timestep];
		//				Tile* myPrevious = path[timestep - 1];
		//				otherAgentCurrent->color = vec3(1, 1, 0);
		//				myPrevious->color = vec3(1, 0, 1);
		//				if (otherAgentCurrent == myPrevious)
		//				{
		//					std::cout << "Strange collision" << std::endl;
		//					collisions.insert(CollisionData(agent, currentTile, timestep + Simulation::timestep));
		//					collisions.insert(CollisionData(otherAgent, otherAgentCurrent, timestep + Simulation::timestep));
		//				}
		//			}
		//		}
		//	}
		//}
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

void AgentCoordinator::FindAdjustedPath(AStar::Path& path)
{
	
}

void AgentCoordinator::UpdateAgents(std::vector<Agent*> agents)
{
	PopTimestep();

	// update the reservation table with any new paths
	for (Agent* agent : agents)
	{
		if (agent->hasReachedGoal)
		{
			Tile* currentTile = map->getTileAt(agent->x, agent->y);
			Tile* goalTile = map->randomWalkableTile();
			agent->setPath(aStar->findPath(currentTile, goalTile));
			AddPath(agent, agent->path);
		}

		agent->step();
	}

	

	//////////////////////////////////////////////////////////////////////////

	// check for collisions by seeing if any tile is being used by more than one path
	// at a timestep t
	for (int t = 0; t < tileToPathMapAtTimestep.size(); t++)
	{
		TileToPathMap& tilePathMap = tileToPathMapAtTimestep[t];

		TileToPathMap::iterator it;
		for (it = tilePathMap.begin(); it != tilePathMap.end(); it++)
		{
			if (it->second.size() > 1) // collision occurs
			{
				std::cout << "Collision at " << *it->first << " at timestep " << t << std::endl;
				float v = 1.f / (float) (t + 1);
				it->first->color = vec3(v, 0, v);
				pathCollisions.push_back(it->second);
			}
		}
	}

	// check for collisions when two agents pass one another
	AgentToPathsMap::iterator it;
	for (it = agentToPathsMap.begin(); it != agentToPathsMap.end(); it++)
	{

		
		Agent* agent = it->first;
		for (AStar::Path path : it->second) // check all our paths
		{
			std::cout << "Checking path of " << path.size() << "||" << tileToPathMapAtTimestep.size() << std::endl;
			for (int timestep = 0; timestep < path.size(); timestep++)
			{
				Tile* currentTile = path[timestep];
				TileToPathMap tileToPathMap = tileToPathMapAtTimestep[timestep];// [currentTile];
			}
		}
	}

}

void AgentCoordinator::DrawPotentialPaths(Graphics* graphics)
{
	graphics->LineBatchBegin();
	AgentToPathsMap::iterator it;
	int numDraws = 0;
	std::vector<ivec3> points;
	for (it = agentToPathsMap.begin(); it != agentToPathsMap.end(); it++)
	{
		Agent* agent = it->first;
		std::vector<AStar::Path>& paths = it->second;

		for (AStar::Path& path : paths)
		{
			for (Tile* tile : path)
				points.emplace_back(vec3(tile->x, tile->y, 0));
			
			numDraws += 1;
			graphics->DrawLine(points, agent->color);//vec3(0, 0, 0));
			points.clear();
		}
	}

	graphics->LineBatchEnd();
}


