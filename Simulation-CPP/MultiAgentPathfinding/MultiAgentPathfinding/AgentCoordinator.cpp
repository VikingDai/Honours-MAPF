#include "AgentCoordinator.h"

#include "Agent.h"
#include "GridMap.h"
#include "AStar.h"
#include "Tile.h"
#include "Simulation.h"

using namespace std;


/* Gives each agent a path*/
void AgentCoordinator::UpdateAgents(vector<Agent*>& agents)
{
	bool anyPathsChanged = false;

	// update the reservation table with any new paths
	for (Agent* agent : agents)
	{
		if (agent->path.empty())
		{
			agent->allPaths.clear();

			Tile* currentTile = map->getTileAt(agent->x, agent->y);
			Tile* goalTile = map->randomWalkableTile();

			/*while (currentTile == goalTile)
			goalTile = map->randomWalkableTile();*/

			AddPath(agent, aStar->findPath(currentTile, goalTile));
			anyPathsChanged = true;
		}
	}

	BuildTable(agents);

	PrintAllPaths(agents);

	// detect collisions and resolve them using the MIP solver
	if (anyPathsChanged)
		ResolveConflicts(agents);

	// we have successfully resolved all conflicts, now agents move along their paths
	for (Agent* agent : agents)
		agent->step();

	PopTimestep();
}

void AgentCoordinator::AddPath(Agent* agent, AStar::Path& path)
{
	assert(!path.empty());

	int newSize = path.size();

	if (tileToPathMapAtTimestep.size() < newSize)
		tileToPathMapAtTimestep.resize(newSize);

	// associate the path to the agent
	agent->allPaths.push_back(path);

	//// associate each tile in the path to a timestep and pointer to the path
	//for (int timestep = 0; timestep < path.size(); timestep++)
	//{
	//	Tile* currentTile = path[timestep];
	//	TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
	//	vector<AgentPath>& pathsUsingTile = tilePathMap[currentTile];
	//	pathsUsingTile.push_back(AgentPath(agent, &path));
	//}

	for (Tile* tile : path)
	{
		std::cout << "Agent: " << agent->getAgentId() << *tile << std::endl;
	}
}

AgentCoordinator::AgentCoordinator(GridMap* inMap)
{
	map = inMap;
	aStar = new AStar(inMap);
}


/* Check if any paths are in collision ~ that is they share the same tile at the same timestep */
vector<set<AStar::Path*>> AgentCoordinator::CheckCollisions(vector<Agent*>& agents)
{
	vector<set<AStar::Path*>> pathCollisions;

	for (int t = 0; t < tileToPathMapAtTimestep.size(); t++)
	{
		TileToPathMap& tilePathMap = tileToPathMapAtTimestep[t];

		TileToPathMap::iterator it;
		for (it = tilePathMap.begin(); it != tilePathMap.end(); it++)
		{
			if (it->second.size() > 1) // collision occurs
			{
				cout << "Collision at " << *it->first << " at timestep " << t << endl;
				float v = 1.f / (float) (t + 1);
				it->first->color = vec3(v, 0, v);

				// get all paths involved in this collision
				set<AStar::Path*> pathsInvolved;
				for (AgentPath agentPath : it->second)
					pathsInvolved.emplace(agentPath.path);

				pathCollisions.push_back(pathsInvolved);
			}
		}
	}

	// check for collisions when two agents pass one another
	for (Agent* agent : agents)
	{
		//Agent* agent = it->first;
		for (AStar::Path& path : agent->allPaths) // check all our paths
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

					std::cout << "Checking " << *agent  << " against " << *agentPath.agent << std::endl;

					if (!agentPath.path)
					{
						cerr << "PATH NOT VALID" << endl;
						continue;
					}

					AStar::Path& otherPath = (*agentPath.path);

					if (timestep >= otherPath.size()) continue;

					Tile* currentTileOther = otherPath[timestep];
					Tile* previousTileOther = timestep > 0 ? otherPath[timestep - 1] : map->getTileAt(agentPath.agent->x, agentPath.agent->y);

					cout << "\t\t" << *currentTile << "  ||  " << *previousTileOther << endl;

					assert(currentTileOther == previousTile); // the line where we set paths should have made this true

					// there is a collision if two agents 'swap' position
					if (currentTile == previousTileOther && previousTile == currentTileOther)
					{
						float v = 1.f / (float) (timestep + 1);
						currentTile->color = vec3(v, 0, v);
						previousTile->color = vec3(v, 0, v);

						set<AStar::Path*> pathsInvolved = { agentPath.path, &path };
						pathCollisions.push_back(pathsInvolved);

						cout << "COLLISION!" << endl;
					}
				}

				previousTile = currentTile;
			}
		}
	}

	return pathCollisions;
}


void AgentCoordinator::DrawPotentialPaths(Graphics* graphics, vector<Agent*> agents)
{
	graphics->LineBatchBegin();

	int numDraws = 0;
	vector<ivec3> points;
	for (Agent* agent : agents)
	{
		for (AStar::Path& path : agent->allPaths)
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
		for (AStar::Path& path : agent->allPaths)
			longestPathSize = std::max(longestPathSize, (int) path.size());
	}

	tileToPathMapAtTimestep.resize(longestPathSize);


	// add these paths to the table
	for (Agent* agent : agents)
	{
		for (AStar::Path& path : agent->allPaths)
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

	//int longestPathSize = -1;

	//// get the largest path size
	//for (Agent* agent : agents)
	//{
	//	
	//	for (AStar::Path& path : agent->allPaths)
	//		longestPathSize = std::max(longestPathSize, (int) path.size());
	//}
	//
	//for (Agent* agent : agents)
	//{
	//	// pad any agent's paths to match that of the longest path size
	//	for (AStar::Path& path : agent->allPaths)
	//	{
	//		for (int i = path.size(); i < longestPathSize; i++)
	//		{
	//			Tile* lastTile = path.back();
	//			path.push_back(lastTile);
	//		}
	//	}

	//	// add these paths to the table
	//	for (AStar::Path& path : agent->allPaths)
	//	{
	//		for (int timestep = 0; timestep < path.size(); timestep++)
	//		{
	//			Tile* currentTile = path[timestep];
	//			TileToPathMap& tilePathMap = tileToPathMapAtTimestep[timestep];
	//			vector<AgentPath>& pathsUsingTile = tilePathMap[currentTile];
	//			pathsUsingTile.push_back(AgentPath(agent, &path));
	//		}
	//	}
	//}
}


void AgentCoordinator::PrintAllPaths(std::vector<Agent*>& agents)
{
	for (Agent* agent : agents)
	{
		for (AStar::Path& path : agent->allPaths)
		{
			cout << "Path for agent: " << agent->getAgentId() << endl;

			for (Tile* tile : path)
			{
				std::cout << "\t" << *tile << std::endl;
			}
		}
	}
}

SCIP_RETCODE AgentCoordinator::SetupProblem(SCIP* scip, vector<Agent*>& agents)
{
	// create empty problem
	SCIP_CALL_EXC(SCIPcreateProbBasic(scip, "string"));

	const SCIP_Real NEG_INFINITY = -SCIPinfinity(scip);
	const SCIP_Real POS_INFINITY = SCIPinfinity(scip);

	// create variables for paths as well as penalties and constraints
	std::map<AStar::Path*, SCIP_VAR*> pathToSCIPvarMap;

	for (Agent* agent : agents)
	{
		vector<SCIP_VAR*> agentVariables;

		// get the agent's id
		int agentId = agent->getAgentId();

		vector<AStar::Path>& paths = agent->allPaths;//it->second;
		for (int i = 0; i < paths.size(); i++) // for each path construct a variable in the form 'a1p1'
		{
			AStar::Path& path = paths[i];
			assert(!path.empty());

			// create variable describing path
			SCIP_VAR* pathVar;
			char pathVarName[50];
			sprintf(pathVarName, "a%dp%d", agentId, i);

			int pathSize = path.size();
			SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &pathVar, pathVarName, 0, 1, pathSize, SCIP_VARTYPE_INTEGER));
			SCIP_CALL_EXC(SCIPaddVar(scip, pathVar));

			// add it to the map
			pathToSCIPvarMap[&path] = pathVar;
			varToPathMap[pathVar] = &path;
			varToAgentMap[pathVar] = agent;

			allVariables.push_back(pathVar);
			agentVariables.push_back(pathVar);

			// create variable describing penalty
			SCIP_VAR* penaltyVar;
			char penaltyVarName[50];
			sprintf(penaltyVarName, "a%dq%d", agentId, i);
			SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &penaltyVar, penaltyVarName, 0, 1, 1000, SCIP_VARTYPE_INTEGER));
			SCIP_CALL_EXC(SCIPaddVar(scip, penaltyVar));
			varToAgentMap[penaltyVar] = agent;

			allVariables.push_back(penaltyVar);
			agentVariables.push_back(penaltyVar);
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
	vector<set<AStar::Path*>> pathCollisions = CheckCollisions(agents);

	for (set<AStar::Path*>& paths : pathCollisions)
	{
		SCIP_CONS* collisionCons;
		char collisionConsName[50];
		sprintf_s(collisionConsName, "collision%d", collisionCount);
		SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &collisionCons, collisionConsName, 0, nullptr, nullptr, 1, 1));

		// apply collision constraints to path variables
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

void AgentCoordinator::ResolveConflicts(vector<Agent*>& agents)
{
	SCIP* scip;
	SCIP_CALL_EXC(SCIPcreate(&scip));
	SCIP_CALL_EXC(SCIPincludeDefaultPlugins(scip));

	SCIPinfoMessage(scip, nullptr, "\n");
	SCIPinfoMessage(scip, nullptr, "****************************\n");
	SCIPinfoMessage(scip, nullptr, "* Running MAPF SCIP Solver *\n");
	SCIPinfoMessage(scip, nullptr, "****************************\n");
	SCIPinfoMessage(scip, nullptr, "\n");

	SCIP_CALL_EXC(SetupProblem(scip, agents));

	//SCIPinfoMessage(scip, nullptr, "Original problem:\n");
	//SCIP_CALL_EXC(SCIPprintOrigProblem(scip, nullptr, "cip", FALSE));

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
		//	cout << "xNum: " << solution << endl;
		for (SCIP_VAR* var : allVariables)
		{
			if (!var) continue;

			double varSolution = SCIPgetSolVal(scip, Solution, var);

			// has chosen this path
			if (varSolution == 1)
			{
				Agent* agent = varToAgentMap[var];

				bool isPathVariable = varToPathMap.count(var);
				if (isPathVariable)
				{
					AStar::Path& path = *varToPathMap[var];
					agent->setPath(path);
				}
				else // the variable is a penalty var
				{
					agent->setPath(AStar::Path{ map->getTileAt(agent->x, agent->y) });
					cout << "Failed to find a solution!" << endl;
				}
			}


			/*if (var->name)
				cout << "solution for: " << var->name << " = " << varSolution << endl;
			else
				cout << "solution name not valid for var with " << varSolution << endl;*/
		}
	}

	// release variables
	for (SCIP_VAR* var : allVariables)
		SCIP_CALL_EXC(SCIPreleaseVar(scip, &var));

	allVariables.clear();
	varToAgentMap.clear();
	varToPathMap.clear();
	varNames.clear();

	SCIP_CALL_EXC(SCIPfree(&scip));
}