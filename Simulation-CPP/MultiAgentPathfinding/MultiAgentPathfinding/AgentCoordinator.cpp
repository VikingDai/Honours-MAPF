#include "AgentCoordinator.h"

#include "Agent.h"
#include "GridMap.h"
#include "AStar.h"
#include "Tile.h"
#include "Simulation.h"


void AgentCoordinator::AddPath(Agent* agent, AStar::Path& path)
{
	if (path.empty()) return;

	int newSize = path.size();

	if (tileToPathMapAtTimestep.size() < newSize)
		tileToPathMapAtTimestep.resize(newSize);

	// associate the path to the agent
	if (std::find(agentToPathsMap[agent].begin(), agentToPathsMap[agent].end(), &path) == agentToPathsMap[agent].end())
		agentToPathsMap[agent].push_back(&path);

	for (int timestep = 0; timestep < path.size(); timestep++)
	{
		// associate each tile in the path to a timestep and pointer to the path
		Tile* currentTile = path[timestep];
		TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
		std::vector<AgentPath>& pathsUsingTile = tilePathMap[currentTile];
		pathsUsingTile.push_back(AgentPath(agent, &path));

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
	// update the reservation table with any new paths
	for (Agent* agent : agents)
	{
		if (agent->hasReachedGoal)
		{
			Tile* currentTile = map->getTileAt(agent->x, agent->y);
			Tile* goalTile = map->randomWalkableTile();

			while (currentTile == goalTile)
				goalTile = map->randomWalkableTile();

			agent->setPath(aStar->findPath(currentTile, goalTile));
			AddPath(agent, agent->path);

			/*std::cout << *agent << std::endl;
			for (Tile* tile : agent->path)
				std::cout << *tile << " > ";
			std::cout << std::endl;*/
		}
	}
	AgentToPathsMap::iterator it;
	for (it = agentToPathsMap.begin(); it != agentToPathsMap.end(); it++)
	{
		Agent* agent = it->first;
		std::cout << *agent << ":" << std::endl;
		for (AStar::Path* path : it->second)
			for (Tile* tile : *path)
				std::cout << '\t' << *tile << std::endl;
	}


	CheckCollisions();

	// #TODO pass the paths and which paths are in collisions to the solver
	//ScipSolve();

	// we have successfully resolved all conflicts, now agents move along their paths
	for (Agent* agent : agents)
	{
		agent->step();
	}

	PopTimestep();
}

std::vector<std::set<AStar::Path*>> AgentCoordinator::CheckCollisions()
{
	std::vector<std::set<AStar::Path*>> pathCollisions;

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

				// get all paths involved in this collision
				std::set<AStar::Path*> pathsInvolved;
				for (AgentPath agentPath : it->second)
					pathsInvolved.emplace(agentPath.path);

				pathCollisions.push_back(pathsInvolved);
			}
		}
	}

	// check for collisions when two agents pass one another
	AgentToPathsMap::iterator it;
	for (it = agentToPathsMap.begin(); it != agentToPathsMap.end(); it++)
	{
		Agent* agent = it->first;
		for (AStar::Path* path : it->second) // check all our paths
		{
			if (path->empty()) continue;

			Tile* previousTile = map->getTileAt(agent->x, agent->y);

			for (int timestep = 0; timestep < path->size(); timestep++)
			{
				Tile* currentTile = (*path)[timestep];

				// check all paths on this tile at the current timestep
				TileToPathMap& tileToPathMap = tileToPathMapAtTimestep[timestep];
				std::vector<AgentPath>& paths = tileToPathMap[previousTile];
				for (AgentPath& agentPath : paths) // check agents using our previous tile at the current timestep
				{
					if (agentPath.agent == agent) continue; // skip any paths using the same agent

					AStar::Path& otherPath = (*agentPath.path);

					Tile* currentTileOther = otherPath[timestep];
					Tile* previousTileOther = timestep > 0 ? otherPath[timestep - 1] : map->getTileAt(agentPath.agent->x, agentPath.agent->y);

					assert(currentTileOther == previousTile); // the line where we set paths should have made this true

					std::cout << "\t\t" << *currentTile << "  ||  " << *previousTileOther << std::endl;

					// there is a collision if two agents 'swap' position
 					if (currentTile == previousTileOther && previousTile == currentTileOther)
					{
						float v = 1.f / (float) (timestep + 1);
						currentTile->color = vec3(v, 0, v);
						previousTile->color = vec3(v, 0, v);

						std::set<AStar::Path*> pathsInvolved = { agentPath.path, path };
						pathCollisions.push_back(pathsInvolved);

						std::cout << "COLLISION!" << std::endl;
					}
				}

				previousTile = currentTile;
			}
		}
	}

	return pathCollisions;
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
		std::vector<AStar::Path*> paths = it->second;

		for (AStar::Path* path : paths)
		{
			for (Tile* tile : *path)
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

SCIP_RETCODE AgentCoordinator::SetupProblem(SCIP* scip)
{
	// create empty problem
	SCIP_CALL_EXC(SCIPcreateProbBasic(scip, "string"));

	const SCIP_Real NEG_INFINITY = -SCIPinfinity(scip);
	const SCIP_Real POS_INFINITY = SCIPinfinity(scip);

	//////////////////////////////////////////////////////////////////////////
	// create variables for paths as well as penalties and constraints
	std::map<AStar::Path*, SCIP_VAR*> pathToSCIPvarMap;

	AgentToPathsMap::iterator it;
	for (it = agentToPathsMap.begin(); it != agentToPathsMap.end(); it++)
	{
		std::vector<SCIP_VAR*> agentVariables;

		int agentId = it->first->getAgentId(); // get the agent's id
		std::vector<AStar::Path*>& paths = it->second;
		for (int i = 0; i < paths.size(); i++) // for each path construct a variable in the form 'a1p1'
		{
			// create variable describing path
			SCIP_VAR* pathVar;
			char pathVarName[50];
			sprintf(pathVarName, "a%dp%d", agentId, i);

			int pathSize = paths[i]->size();
			SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &pathVar, pathVarName, 0, 1, pathSize, SCIP_VARTYPE_INTEGER));
			SCIP_CALL_EXC(SCIPaddVar(scip, pathVar));
			
			// add it to the map
			pathToSCIPvarMap[paths[i]] = pathVar;

			// create variable describing penalty
			SCIP_VAR* penaltyVar;
			char penaltyVarName[50];
			sprintf(penaltyVarName, "a%dq%d", agentId, i);
			SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &penaltyVar, penaltyVarName, 0, 1, 1000, SCIP_VARTYPE_INTEGER));
			SCIP_CALL_EXC(SCIPaddVar(scip, penaltyVar));

			allVariables.push_back(pathVar);
			allVariables.push_back(penaltyVar);

			agentVariables.push_back(pathVar);
			agentVariables.push_back(penaltyVar);
		}

		// create a constraint describing that an agent can pick one path or a penalty
		SCIP_CONS* agentChoiceCons;
		char choiceConsName[50];
		sprintf(choiceConsName, "agentChoice%d", agentId);
		SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &agentChoiceCons, choiceConsName, 0, nullptr, nullptr, 1, 1));
		for (SCIP_VAR* var : agentVariables)
			SCIP_CALL_EXC(SCIPaddCoefLinear(scip, agentChoiceCons, var, 1.0));

		// add then release constraint
		SCIP_CALL_EXC(SCIPaddCons(scip, agentChoiceCons));
		SCIP_CALL_EXC(SCIPreleaseCons(scip, &agentChoiceCons));
	}

	//////////////////////////////////////////////////////////////////////////
	// #TODO create constraints for path collisions
	std::vector<std::set<AStar::Path*>> pathCollisions = CheckCollisions();


	int collisionCount = 0;
	for (std::set<AStar::Path*>& paths : pathCollisions)
	{
		SCIP_CONS* collisionCons;
		char collisionConsName[50];
		sprintf(collisionConsName, "collision%d", collisionCount);
		SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &collisionCons, collisionConsName, 0, nullptr, nullptr, 1, 1));

		for (AStar::Path* path : paths)
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

void AgentCoordinator::ScipSolve()
{
	SCIP* scip;
	SCIP_CALL_EXC(SCIPcreate(&scip));
	SCIP_CALL_EXC(SCIPincludeDefaultPlugins(scip));

	SCIPinfoMessage(scip, nullptr, "\n");
	SCIPinfoMessage(scip, nullptr, "****************************\n");
	SCIPinfoMessage(scip, nullptr, "* Running MAPF SCIP Solver *\n");
	SCIPinfoMessage(scip, nullptr, "****************************\n");
	SCIPinfoMessage(scip, nullptr, "\n");

	SCIP_CALL_EXC(SetupProblem(scip));

	SCIPinfoMessage(scip, nullptr, "Original problem:\n");
	SCIP_CALL_EXC(SCIPprintOrigProblem(scip, nullptr, "cip", FALSE));

	SCIPinfoMessage(scip, nullptr, "\n");
	SCIP_CALL_EXC(SCIPpresolve(scip));

	SCIPinfoMessage(scip, nullptr, "\nSolving...\n");
	SCIP_CALL_EXC(SCIPsolve(scip));

	SCIP_CALL_EXC(SCIPfreeTransform(scip));

	if (SCIPgetNSols(scip) > 0)
	{
		SCIPinfoMessage(scip, nullptr, "\nSolution:\n");
		SCIP_CALL_EXC(SCIPprintSol(scip, SCIPgetBestSol(scip), nullptr, FALSE));

		SCIP_SOL* Solution = SCIPgetBestSol(scip);

		//	double solution = SCIPgetSolVal(scip, Solution, xNum);
		//	std::cout << "xNum: " << solution << std::endl;
	}

	// release variables
	for (SCIP_VAR* var : allVariables)
		SCIP_CALL_EXC(SCIPreleaseVar(scip, &var));
	allVariables.clear();

	SCIP_CALL_EXC(SCIPfree(&scip));
}