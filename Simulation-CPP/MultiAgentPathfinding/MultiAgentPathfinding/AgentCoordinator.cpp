#include "AgentCoordinator.h"

#include "Agent.h"
#include "GridMap.h"
#include "TemporalAStar.h"
#include "Tile.h"
#include "Simulation.h"
//#include "Options.h"

#include <chrono>
#include <ctime>
#include "Statistics.h"
#include <sstream>
#include <string>

#define DEBUG_MIP 0

using namespace std;

AgentCoordinator::AgentCoordinator(GridMap* inMap)
{
	map = inMap;
	aStar = new TemporalAStar(inMap);
	pathAssigner = new PathAssigner(inMap);
}

void AgentCoordinator::Reset()
{

}

/* Gives each agent a path*/
void AgentCoordinator::UpdateAgents(vector<Agent*>& agents)
{
	set<Agent*> agentsWhoNeedNewPaths;
	std::map<Agent*, TileCollision> agentCollisionMap;

	bool anyPathsChanged = false;

	// if the agent has a goal to reach but does not have a path, generate a path
	for (Agent* agent : agents)
	{
		if (agent->goal && agent->getPath().empty())
		{
			agentsWhoNeedNewPaths.emplace(agent);
			agent->allPaths.clear();
			anyPathsChanged = true;
		}
	}

	if (!anyPathsChanged) return; // only assign new paths if any agents require them

	bool firstRun = true;

	// detect collisions and resolve them using the MIP solver
	coordinatorTimer.Begin();

	int i = 0;
	do
	{
		std::cout << "************************" << std::endl;
		std::cout << "**** RESOLVING RUN " << i << " ****" << std::endl;
		std::cout << "************************" << std::endl;

		// Generate additional paths
		Timer timerGeneratePath;
		timerGeneratePath.Begin();
		for (Agent* agent : agentsWhoNeedNewPaths)
		{
			std::cout << "Generating path for " << *agent << std::endl;
			GeneratePath(agent, firstRun, agentCollisionMap);
		}
		timerGeneratePath.End();
		timerGeneratePath.PrintTimeElapsed("Generating paths");

		agentsWhoNeedNewPaths.clear();

		// Build collision table
		Timer timerBuildTable;
		timerBuildTable.Begin();
		std::cout << "--- Building Table ---" << std::endl;
		BuildTable(agents);
		timerBuildTable.End();
		timerBuildTable.PrintTimeElapsed("Building collision table");

		PathCollisions collisions = CheckCollisions(agents, agentCollisionMap);

		// Assign conflict-free paths to agents using a MIP
		mipTimer.Begin();
		std::vector<Agent*> mipConflicts = pathAssigner->AssignPaths(agents, collisions, pathLengths);//ResolveConflicts(agents, collisions);
		mipTimer.End();
		mipTimer.PrintTimeElapsed("MIP Path assignment");
		Stats::avgMipTime = mipTimer.GetAvgTime();
		
		if (mipConflicts.empty())
			std::cout << "MIP told us no conflicts, we are done!" << std::endl;

		for (Agent* agent : mipConflicts)
			agentsWhoNeedNewPaths.emplace(agent);

		if (!agentsWhoNeedNewPaths.empty())
		{
			std::map<Agent*, TileCollision>::iterator it;
			for (it = agentCollisionMap.begin(); it != agentCollisionMap.end(); it++)
				agentsWhoNeedNewPaths.emplace(it->first);
		}

		i++;

		firstRun = false;
	}
	while (!agentsWhoNeedNewPaths.empty());

	coordinatorTimer.End();
	Stats::avgCoordinatorTime = coordinatorTimer.GetAvgTime();
	coordinatorTimer.PrintTimeElapsed("Agent Coordinator");
}

void AgentCoordinator::GeneratePath(Agent* agent, bool bfirstRun, std::map<Agent*, TileCollision> agentCollisionMap)
{
	Tile* currentTile = map->getTileAt(agent->x, agent->y);
	if (!agent->goal)
		agent->goal = map->randomWalkableTile();

	// check any collisions in the path and update the custom costs table
	TemporalAStar::TileCosts customCosts;
	for (auto& it : agentCollisionMap[agent])
	{
		Tile* tile = it.first;
		int time = it.second;
		customCosts[time][tile] += 1;

	}

	if (bfirstRun)
	{
		//TemporalAStar::Path& path = aStar->FindPath(currentTile, agent->goal);
		TemporalAStar::Path& path = aStar->FindPath2(currentTile, agent->goal);
		//TemporalAStar::Path& path = agent->bfs->FindNextPath(currentTile, agent->goal); //aStar->FindPath(currentTile, agent->goal);

		// give the path to the agent
		agent->allPaths.push_back(path);
	}
	else
	{
		//for (int i = 0; i < 10; i++)
		{
			//TemporalAStar::Path& path = aStar->FindPath(currentTile, agent->goal, customCosts);
			TemporalAStar::Path& path = aStar->FindPath2(currentTile, agent->goal, customCosts);
			//TemporalAStar::Path& path = agent->bfs->FindNextPath(currentTile, agent->goal);//aStar->FindPath(currentTile, agent->goal, customCosts);

			// associate the path to the agent
			agent->allPaths.push_back(path);
		}
	}



}



/* Check if any paths are in collision AND maps agents to tile collisions */
vector<set<TemporalAStar::Path*>> AgentCoordinator::CheckCollisions(vector<Agent*>& agents, std::map<Agent*, TileCollision>& agentsInCollision)
{
	vector<set<TemporalAStar::Path*>> pathCollisions;

	for (int t = 0; t < tileToPathMapAtTimestep.size(); t++)
	{
		TileToPathMap& tilePathMap = tileToPathMapAtTimestep[t];

		TileToPathMap::iterator it;
		for (it = tilePathMap.begin(); it != tilePathMap.end(); it++)
		{
			// count unique agents with paths on this tile
			std::set<Agent*> seenAgents;
			for (AgentPath& agentPath : it->second)
			{
				seenAgents.emplace(agentPath.agent);
			}

			if (seenAgents.size() > 1)
			{
				for (Agent* agent : seenAgents)
				{
					agentsInCollision[agent].push_back(std::pair<Tile*, int>(it->first, t));
					//std::cout << "SAME TILE collision: " << *it->first << " Time: " << t << *agent << std::endl;
				}

				// blend color as collision gets closer to occuring
				float v = 1.f / (float) (t + 1);
				it->first->color = vec3(v, 0, v);

				// get all paths involved in this collision
				set<TemporalAStar::Path*> pathsInvolved;
				for (AgentPath agentPath : it->second)
					pathsInvolved.emplace(agentPath.path);

				pathCollisions.push_back(pathsInvolved);
			}
		}
	}

	// check for collisions when two agents pass one another
	for (Agent* agent : agents)
	{
		for (TemporalAStar::Path& path : agent->allPaths) // check all our paths
		{
			if (path.empty()) continue;

			Tile* previousTile = map->getTileAt(agent->x, agent->y);

			for (int timestep = 0; timestep < path.size(); timestep++)
			{
				Tile* currentTile = path[timestep];

				// check all paths on this tile at the current timestep
				TileToPathMap& tileToPathMap = tileToPathMapAtTimestep[timestep];
				vector<AgentPath>& paths = tileToPathMap[previousTile];
				for (AgentPath& agentPath : paths) // check agents using our previous tile at the current timestep
				{
					if (agentPath.agent == agent) continue; // skip any paths using the same agent

					//std::cout << "Checking " << *agent << " against " << *agentPath.agent << std::endl;

					if (!agentPath.path)
					{
						cerr << "PATH NOT VALID" << endl;
						continue;
					}

					TemporalAStar::Path& otherPath = (*agentPath.path);

					if (timestep >= otherPath.size()) continue;

					Tile* currentTileOther = otherPath[timestep];
					Tile* previousTileOther = timestep > 0 ? otherPath[timestep - 1] : map->getTileAt(agentPath.agent->x, agentPath.agent->y);

					assert(currentTileOther == previousTile); // the line where we set paths should have made this true

					// there is a collision if two agents 'swap' position
					if (currentTile == previousTileOther && previousTile == currentTileOther)
					{
						float v = 1.f / (float) (timestep + 1);
						currentTile->color = vec3(v, 0, v);
						previousTile->color = vec3(v, 0, v);

						set<TemporalAStar::Path*> pathsInvolved = { agentPath.path, &path };
						pathCollisions.push_back(pathsInvolved);

						//agentsInCollision[agentPath.agent].emplace(std::pair<Tile*, int>(previousTile, timestep));
						agentsInCollision[agent].push_back(std::pair<Tile*, int>(currentTile, timestep));

						//std::cout << "SWAP collision: " << *previousTile << " Time: " << timestep << *agentPath.agent << std::endl;
						//std::cout << "SWAP collision: " << *currentTile << " Time: " << timestep << *agent << std::endl;

						//agentsInCollision[agentPath.agent].push_back(std::pair<Tile*, int>(currentTile, timestep - 1));
						//agentsInCollision[agent].push_back(std::pair<Tile*, int>(previousTile, timestep - 1));
					}
				}

				previousTile = currentTile;
			}
		}
	}

	return pathCollisions;
}


std::vector<std::pair<Tile*, int>> AgentCoordinator::TilesInCollision(Agent* agent, TemporalAStar::Path& path)
{
	std::vector<std::pair<Tile*, int>> tilesInCollision;

	for (int timestep = 0; timestep < path.size(); timestep++)
	{
		Tile* tile = path[timestep];

		vector<AgentPath> agentsOnTile = tileToPathMapAtTimestep[timestep][tile];
		// count unique agents with paths on this tile
		std::set<Agent*> seenAgents;
		for (AgentPath& agentPath : agentsOnTile)
			seenAgents.emplace(agentPath.agent);

		if (seenAgents.size() > 1)
			tilesInCollision.push_back(std::pair<Tile*, int>(tile, timestep));
	}

	// check for collisions when two agents pass one another
	Tile* previousTile = map->getTileAt(agent->x, agent->y);

	for (int timestep = 0; timestep < path.size(); timestep++)
	{
		Tile* tile = path[timestep];

		// check all paths on this tile at the current timestep
		TileToPathMap& tileToPathMap = tileToPathMapAtTimestep[timestep];
		vector<AgentPath>& paths = tileToPathMap[previousTile];
		for (AgentPath& agentPath : paths) // check agents using our previous tile at the current timestep
		{
			if (agentPath.agent == agent) continue; // skip any paths using the same agent

			if (!agentPath.path)
			{
				cerr << "PATH NOT VALID" << endl;
				continue;
			}

			TemporalAStar::Path& otherPath = (*agentPath.path);

			if (timestep >= otherPath.size()) continue;

			Tile* currentTileOther = otherPath[timestep];
			Tile* previousTileOther = timestep > 0 ? otherPath[timestep - 1] : map->getTileAt(agentPath.agent->x, agentPath.agent->y);

			assert(currentTileOther == previousTile); // the line where we set paths should have made this true
													  // there is a collision if two agents 'swap' position

			if (tile == previousTileOther && previousTile == currentTileOther)
			{
				float v = 1.f / (float) (timestep + 1);
				tile->color = vec3(v, 0, v);
				previousTile->color = vec3(v, 0, v);

				tilesInCollision.push_back(std::pair<Tile*, int>(tile, timestep));
			}
		}

		previousTile = tile;
	}

	return tilesInCollision;
}

void AgentCoordinator::DrawPotentialPaths(Graphics* graphics, vector<Agent*> agents)
{
	graphics->LineBatchBegin();

	int numDraws = 0;
	vector<vec3> points;
	for (Agent* agent : agents)
	{
		for (TemporalAStar::Path& path : agent->allPaths)
		{
			for (Tile* tile : path)
				points.emplace_back(vec3(tile->x, tile->y, 0));

			numDraws += 1;

			bool pathHasCollision = false;//pathCollisions.find(path) != pathCollisions.end();
			vec3 color = pathHasCollision ? vec3(1, 0, 0) : vec3(0, 1, 0);
			float lineWidth = pathHasCollision ? 4.f : 1.5f;

			graphics->DrawLine(points, color, lineWidth);
			points.clear();
		}
	}

	graphics->LineBatchEnd();
}

void AgentCoordinator::BuildTable(std::vector<Agent*>& agents)
{
	tileToPathMapAtTimestep.clear();

	// get the largest path size and resize the table to fit
	int longestPathSize = -1;
	for (Agent* agent : agents)
		for (TemporalAStar::Path& path : agent->allPaths)
			longestPathSize = std::max(longestPathSize, (int) path.size());
	tileToPathMapAtTimestep.resize(longestPathSize);

	// add these paths to the table
	for (Agent* agent : agents)
	{
		for (TemporalAStar::Path& path : agent->allPaths)
		{
			int lastIndex = path.size() - 1;
			for (int timestep = 0; timestep < longestPathSize; timestep++)
			{
				Tile* currentTile = timestep <= lastIndex ? path[timestep] : path[lastIndex];
				TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
				vector<AgentPath>& pathsUsingTile = tilePathMap[currentTile];
				pathsUsingTile.push_back(AgentPath(agent, &path));
			}

			/*for (int timestep = 0; timestep < path.size(); timestep++)
			{
				Tile* currentTile = path[timestep];
				TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
				vector<AgentPath>& pathsUsingTile = tilePathMap[currentTile];
				pathsUsingTile.push_back(AgentPath(agent, &path));
			}*/
		}
	}
}

void AgentCoordinator::PrintAllPaths(std::vector<Agent*>& agents)
{
	for (Agent* agent : agents)
		for (TemporalAStar::Path& path : agent->allPaths)
			PrintPath(agent, path);
}

void AgentCoordinator::PrintPath(Agent* agent, TemporalAStar::Path& path)
{
	cout << "Path for " << *agent << " of length " << path.size() << " | ";
	for (Tile* tile : path)
		std::cout << *tile << " > ";
	std::cout << std::endl;
}