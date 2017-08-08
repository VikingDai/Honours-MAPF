#include "AgentCoordinator.h"

#include "Agent.h"
#include "GridMap.h"
#include "SpatialAStar.h"
#include "Tile.h"
#include "Simulation.h"
//#include "Options.h"

#include <chrono>
#include <ctime>
#include "Statistics.h"


using namespace std;

/* Gives each agent a path*/
void AgentCoordinator::UpdateAgents(vector<Agent*>& agents)
{
	set<Agent*> agentsWhoNeedNewPaths;
	std::map<Agent*, TileCollision> agentCollisionMap;

	bool anyPathsChanged = false;

	// if the agent has a goal to reach but does not have a path, generate a path
	for (Agent* agent : agents)
	{
		if (agent->goal && agent->currentPath.empty())
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

		if (i > 500)
		{
			std::cout << "Exceeded the search limit: failed to resolve path conflicts" << std::endl;
			break;
		}

		// we need to feed the MIP, additional paths
		for (Agent* agent : agentsWhoNeedNewPaths)
		{
			std::cout << "Generating path for " << *agent << std::endl;
			GeneratePath(agent, !firstRun, agentCollisionMap);
		}

		// pad each agents path so that they update the collision table
		for (Agent* agent : agents)
		{
			for (SpatialAStar::Path& path : agent->allPaths)
			{
				PathLengths[&path] = path.size(); // store the paths for use in the MIP

				int paddingRequired = tileToPathMapAtTimestep.size() - path.size();
				for (int p = 0; p < paddingRequired; p++)
					path.push_back(path[path.size() - 1]);

				PrintPath(agent, path);
			}
		}

		// #TODO Make this code less ugly
		agentsWhoNeedNewPaths.clear();

		std::cout << "--- Building Table ---" << std::endl;
		BuildTable(agents);

		std::chrono::time_point<std::chrono::system_clock> TIME_START, TIME_END;
		TIME_START = std::chrono::system_clock::now();


		mipTimer.Begin();
		PathCollisions collisions = CheckCollisions(agents, agentCollisionMap);
		std::vector<Agent*> mipConflicts = ResolveConflicts(agents, collisions);
		mipTimer.End();
		mipTimer.PrintTimeElapsed("SCIP");
		Stats::avgMipTime = mipTimer.GetAvgTime();

		if (mipConflicts.empty())
		{
			std::cout << "MIP told us no conflicts, we are done!" << std::endl;
		}

		for (Agent* agent : mipConflicts)
		{
			agentsWhoNeedNewPaths.emplace(agent);
		}

		if (!agentsWhoNeedNewPaths.empty())
		{
			std::map<Agent*, std::set<std::pair<Tile*, int>>>::iterator it;
			for (it = agentCollisionMap.begin(); it != agentCollisionMap.end(); it++)
			{
				it->second.size();
				agentsWhoNeedNewPaths.emplace(it->first);
			}
		}

		i++;

		firstRun = false;
	}
	while (!agentsWhoNeedNewPaths.empty());

	coordinatorTimer.End();
	Stats::avgCoordinatorTime = coordinatorTimer.GetAvgTime();
	coordinatorTimer.PrintTimeElapsed("Agent Coordinator");
}

void AgentCoordinator::GeneratePath(Agent* agent, bool useCollisions, std::map<Agent*, TileCollision> agentCollisionMap)
{
	Tile* currentTile = map->getTileAt(agent->x, agent->y);
	if (!agent->goal)
		agent->goal = map->randomWalkableTile();

	// check any collisions in the path and update the custom costs table
	SpatialAStar::TileCosts customCosts;
	if (useCollisions)
	{
		for (auto& it : agentCollisionMap[agent])
		{
			Tile* tile = it.first;
			int time = it.second;
			customCosts[time][tile] = 1000;
			//std::cout << "Penalty applied to " << *tile << " at time " << time << " for " << *agent << std::endl;
		}
	}
	//AStar::TileCosts::iterator it;
	//for (it = customCosts.begin(); it != customCosts.end(); it++)
	//{
	//	std::map<Tile*, float>::iterator it2;
	//	//std::cout << "at time: " << it->first << " has penalty ";

	//	for (it2 = it->second.begin(); it2 != it->second.end(); it2++)
	//		std::cout << *it2->first;
	//	std::cout << std::endl;
	//}

	SpatialAStar::Path& path = aStar->FindPath(currentTile, agent->goal, customCosts);

	// associate the path to the agent
	agent->allPaths.push_back(path);
	agent->currentPath = path;
}

AgentCoordinator::AgentCoordinator(GridMap* inMap)
{
	map = inMap;
	aStar = new SpatialAStar(inMap);
}

void AgentCoordinator::Reset()
{

}

/* Check if any paths are in collision AND maps agents to tile collisions */
vector<set<SpatialAStar::Path*>> AgentCoordinator::CheckCollisions(vector<Agent*>& agents, std::map<Agent*, TileCollision>& agentsInCollision)
{
	vector<set<SpatialAStar::Path*>> pathCollisions;

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
					agentsInCollision[agent].emplace(std::pair<Tile*, int>(it->first, t));
					//std::cout << "SAME TILE collision: " << *it->first << " Time: " << t << *agent << std::endl;
				}

				// blend color as collision gets closer to occuring
				float v = 1.f / (float) (t + 1);
				it->first->color = vec3(v, 0, v);

				// get all paths involved in this collision
				set<SpatialAStar::Path*> pathsInvolved;
				for (AgentPath agentPath : it->second)
					pathsInvolved.emplace(agentPath.path);

				pathCollisions.push_back(pathsInvolved);
			}
		}
	}

	// check for collisions when two agents pass one another
	for (Agent* agent : agents)
	{
		for (SpatialAStar::Path& path : agent->allPaths) // check all our paths
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

					SpatialAStar::Path& otherPath = (*agentPath.path);

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

						set<SpatialAStar::Path*> pathsInvolved = { agentPath.path, &path };
						pathCollisions.push_back(pathsInvolved);

						//agentsInCollision[agentPath.agent].emplace(std::pair<Tile*, int>(previousTile, timestep));
						agentsInCollision[agent].emplace(std::pair<Tile*, int>(currentTile, timestep));

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


std::vector<std::pair<Tile*, int>> AgentCoordinator::TilesInCollision(Agent* agent, SpatialAStar::Path& path)
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

			SpatialAStar::Path& otherPath = (*agentPath.path);

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
		for (SpatialAStar::Path& path : agent->allPaths)
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

	int longestPathSize = -1;

	// get the largest path size
	for (Agent* agent : agents)
	{
		for (SpatialAStar::Path& path : agent->allPaths)
			longestPathSize = std::max(longestPathSize, (int) path.size());
	}

	tileToPathMapAtTimestep.resize(longestPathSize);


	// add these paths to the table
	for (Agent* agent : agents)
	{
		for (SpatialAStar::Path& path : agent->allPaths)
		{
			for (int timestep = 0; timestep < path.size(); timestep++)
			{
				Tile* currentTile = path[timestep];
				TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
				vector<AgentPath>& pathsUsingTile = tilePathMap[currentTile];
				pathsUsingTile.push_back(AgentPath(agent, &path));
			}
		}
	}
}

void AgentCoordinator::PrintAllPaths(std::vector<Agent*>& agents)
{
	for (Agent* agent : agents)
		for (SpatialAStar::Path& path : agent->allPaths)
			PrintPath(agent, path);
}

void AgentCoordinator::PrintPath(Agent* agent, SpatialAStar::Path& path)
{
	cout << "Path for " << *agent << " of length " << path.size() << endl;
	//for (Tile* tile : path)
		//std::cout << "\t" << *tile << std::endl;
}

SCIP_RETCODE AgentCoordinator::SetupProblem(SCIP* scip, vector<Agent*>& agents, PathCollisions& pathCollisions)
{
	// create empty problem
	SCIP_CALL_EXC(SCIPcreateProbBasic(scip, "string"));

	const SCIP_Real NEG_INFINITY = -SCIPinfinity(scip);
	const SCIP_Real POS_INFINITY = SCIPinfinity(scip);

	// create variables for paths as well as penalties and constraints
	std::map<SpatialAStar::Path*, SCIP_VAR*> pathToSCIPvarMap;

	for (Agent* agent : agents)
	{
		vector<SCIP_VAR*> agentVariables;

		// get the agent's id
		int agentId = agent->getAgentId();

		// create variable describing penalty when an agent fails to find a path
		SCIP_VAR* penaltyVar;
		char penaltyVarName[50];
		sprintf(penaltyVarName, "a%dq", agentId);
		SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &penaltyVar, penaltyVarName, 0, 1, 1000, SCIP_VARTYPE_INTEGER));
		SCIP_CALL_EXC(SCIPaddVar(scip, penaltyVar));
		varToAgentMap[penaltyVar] = agent;

		allVariables.push_back(penaltyVar);
		agentVariables.push_back(penaltyVar);

		vector<SpatialAStar::Path>& paths = agent->allPaths;//it->second;
		for (int i = 0; i < paths.size(); i++) // for each path construct a variable in the form 'a1p1'
		{
			SpatialAStar::Path& path = paths[i];
			assert(!path.empty());

			// create variable describing path
			SCIP_VAR* pathVar;
			char pathVarName[50];
			sprintf(pathVarName, "a%dp%d", agentId, i);

			int pathSize = PathLengths[&path];//path.size();
			SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &pathVar, pathVarName, 0, 1, pathSize, SCIP_VARTYPE_INTEGER));
			SCIP_CALL_EXC(SCIPaddVar(scip, pathVar));

			// add it to the map
			pathToSCIPvarMap[&path] = pathVar;
			varToPathMap[pathVar] = &path;
			varToAgentMap[pathVar] = agent;

			allVariables.push_back(pathVar);
			agentVariables.push_back(pathVar);
		}

		// create a constraint describing that an agent can pick one path or a penalty
		SCIP_CONS* agentChoiceCons;
		char choiceConsName[50];
		sprintf_s(choiceConsName, "agentChoice%d", agentId);
		SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &agentChoiceCons, choiceConsName, 0, nullptr, nullptr, 1, 1));
		for (SCIP_VAR* agentPath : agentVariables)
			SCIP_CALL_EXC(SCIPaddCoefLinear(scip, agentChoiceCons, agentPath, 1.0));

		// add then release constraint
		SCIP_CALL_EXC(SCIPaddCons(scip, agentChoiceCons));
		SCIP_CALL_EXC(SCIPreleaseCons(scip, &agentChoiceCons));
	}

	// create constraints for path collisions
	int collisionCount = 0;

	for (set<SpatialAStar::Path*>& paths : pathCollisions)
	{
		SCIP_CONS* collisionCons;
		char collisionConsName[50];
		sprintf_s(collisionConsName, "collision%d", collisionCount);
		SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &collisionCons, collisionConsName, 0, nullptr, nullptr, 0, 1));

		// apply collision constraints to path variables
		for (SpatialAStar::Path* path : paths)
		{
			SCIP_VAR* var = pathToSCIPvarMap[path];
			SCIP_CALL_EXC(SCIPaddCoefLinear(scip, collisionCons, var, 1.0));
		}

		// add then release constraint
		SCIP_CALL_EXC(SCIPaddCons(scip, collisionCons));
		SCIP_CALL_EXC(SCIPreleaseCons(scip, &collisionCons));

		collisionCount += 1;
	}

	return SCIP_OKAY;
}

vector<Agent*> AgentCoordinator::ResolveConflicts(vector<Agent*>& agents, PathCollisions& collisions)
{
	SCIP* scip;
	SCIP_CALL_EXC(SCIPcreate(&scip));
	SCIP_CALL_EXC(SCIPincludeDefaultPlugins(scip));

	//SCIPinfoMessage(scip, nullptr, "\n");
	//SCIPinfoMessage(scip, nullptr, "****************************\n");
	//SCIPinfoMessage(scip, nullptr, "* Running MAPF SCIP Solver *\n");
	//SCIPinfoMessage(scip, nullptr, "****************************\n");
	//SCIPinfoMessage(scip, nullptr, "\n");

	SCIP_CALL_EXC(SetupProblem(scip, agents, collisions));

	//SCIPinfoMessage(scip, nullptr, "Original problem:\n");
	//SCIP_CALL_EXC(SCIPprintOrigProblem(scip, nullptr, "cip", FALSE));

	//SCIPinfoMessage(scip, nullptr, "\n");
	SCIP_CALL_EXC(SCIPpresolve(scip));

	//SCIPinfoMessage(scip, nullptr, "\nSolving...\n");
	SCIP_CALL_EXC(SCIPsolve(scip));

	SCIP_CALL_EXC(SCIPfreeTransform(scip));

	// Check if the solution contains any penalty variables
	vector<Agent*> penaltyAgents;

	if (SCIPgetNSols(scip) > 0)
	{
		SCIPinfoMessage(scip, nullptr, "\nSolution:\n");
		SCIP_CALL_EXC(SCIPprintSol(scip, SCIPgetBestSol(scip), nullptr, FALSE));

		SCIP_SOL* Solution = SCIPgetBestSol(scip);

		for (SCIP_VAR* var : allVariables)
		{
			if (!var)
				continue;

			double varSolution = SCIPgetSolVal(scip, Solution, var);


			// has chosen this path
			if (varSolution == 1)
			{
				Agent* agent = varToAgentMap[var];

				bool isPathVariable = varToPathMap.find(var) != varToPathMap.end();
				if (isPathVariable)
				{
					SpatialAStar::Path& path = *varToPathMap[var];
					agent->setPath(path);
				}
				else // the variable is a penalty var
				{
					agent->setPath(SpatialAStar::Path{ map->getTileAt(agent->x, agent->y) });
					cout << "Failed to find a solution!" << endl;

					penaltyAgents.push_back(agent);
				}
			}
		}
	}
	else
	{
		std::cout << "ERROR: SCIP failed to find any solutions" << std::endl;
	}

	// release variables
	for (SCIP_VAR* var : allVariables)
		SCIP_CALL_EXC(SCIPreleaseVar(scip, &var));

	allVariables.clear();
	varToAgentMap.clear();
	varToPathMap.clear();
	varNames.clear();

	SCIP_CALL_EXC(SCIPfree(&scip));

	if (penaltyAgents.empty())
	{
		std::cout << "EMPTY" << std::endl;
	}

	return penaltyAgents;
}