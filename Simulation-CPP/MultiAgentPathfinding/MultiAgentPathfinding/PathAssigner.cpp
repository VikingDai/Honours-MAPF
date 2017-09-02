#include "PathAssigner.h"

#include "Statistics.h"
#include <sstream>
#include <string>

#define DEBUG_MIP 1

PathAssigner::PathAssigner(GridMap* inGridMap)
{
	map = inGridMap;
}

PathAssigner::~PathAssigner()
{
}

SCIP_RETCODE PathAssigner::SetupProblem(SCIP* scip, std::vector<Agent*>& agents, PathCollisions& pathCollisions, std::map<TemporalAStar::Path*, int>& PathLengths)
{
	// create empty problem
	SCIP_CALL_EXC(SCIPcreateProbBasic(scip, "string"));

	const SCIP_Real NEG_INFINITY = -SCIPinfinity(scip);
	const SCIP_Real POS_INFINITY = SCIPinfinity(scip);

	// create variables for paths as well as penalties and constraints
	std::map<TemporalAStar::Path*, SCIP_VAR*> pathToSCIPvarMap;

	for (Agent* agent : agents)
	{
		std::vector<SCIP_VAR*> agentVariables;

		// get the agent's id
		int agentId = agent->getAgentId();

		// create variable describing penalty when an agent fails to find a path in the form 'a1q'
		SCIP_VAR* penaltyVar;

		std::ostringstream penaltyVarNameStream;
		penaltyVarNameStream << "a" << agentId << "q";
		const char* penaltyVarName = penaltyVarNameStream.str().c_str();

		char penaltyVarNameC[50];
		sprintf(penaltyVarNameC, "a%dq", agentId);

		SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &penaltyVar, penaltyVarNameC, 0, 1, 1000, SCIP_VARTYPE_INTEGER));
		SCIP_CALL_EXC(SCIPaddVar(scip, penaltyVar));
		varToAgentMap[penaltyVar] = agent;
		varNames[penaltyVar] = penaltyVarNameStream.str();

		allVariables.push_back(penaltyVar);
		agentVariables.push_back(penaltyVar);

		std::vector<TemporalAStar::Path>& paths = agent->allPaths;//it->second;
		for (int i = 0; i < paths.size(); i++) // for each path construct a variable in the form 'a1p1'
		{
			TemporalAStar::Path& path = paths[i];
			assert(!path.empty());

			// create variable describing path
			SCIP_VAR* pathVar;

			std::ostringstream pathVarNameStream;
			pathVarNameStream << "a" << agentId << "p" << i;
			const char* pathVarName = pathVarNameStream.str().c_str();

			char pathVarNameC[50];
			sprintf(pathVarNameC, "a%dp%d", agentId, i);

			int pathSize = PathLengths[&path];//path.size();
			SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &pathVar, pathVarNameC, 0, 1, pathSize, SCIP_VARTYPE_INTEGER));
			SCIP_CALL_EXC(SCIPaddVar(scip, pathVar));

			// add it to the map
			pathToSCIPvarMap[&path] = pathVar;
			varToPathMap[pathVar] = &path;
			varToAgentMap[pathVar] = agent;
			varNames[pathVar] = pathVarNameStream.str();

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

	for (std::set<TemporalAStar::Path*>& paths : pathCollisions)
	{
		SCIP_CONS* collisionCons;
		char collisionConsName[50];
		sprintf_s(collisionConsName, "collision%d", collisionCount);
		SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &collisionCons, collisionConsName, 0, nullptr, nullptr, 0, 1));

		// apply collision constraints to path variables
		for (TemporalAStar::Path* path : paths)
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

std::vector<Agent*> PathAssigner::AssignPaths(
	std::vector<Agent*> agents,
	std::vector<std::set<TemporalAStar::Path*>>& collisions,
	std::map<TemporalAStar::Path*, int>& PathLengths)
{
	SCIP* scip;
	SCIP_CALL_EXC(SCIPcreate(&scip));
	SCIP_CALL_EXC(SCIPincludeDefaultPlugins(scip));

	//SCIPinfoMessage(scip, nullptr, "\n");
	//SCIPinfoMessage(scip, nullptr, "****************************\n");
	//SCIPinfoMessage(scip, nullptr, "* Running MAPF SCIP Solver *\n");
	//SCIPinfoMessage(scip, nullptr, "****************************\n");
	//SCIPinfoMessage(scip, nullptr, "\n");

	SCIP_CALL_EXC(SetupProblem(scip, agents, collisions, PathLengths));

	//SCIPinfoMessage(scip, nullptr, "Original problem:\n");
	//SCIP_CALL_EXC(SCIPprintOrigProblem(scip, nullptr, "cip", FALSE));

	//SCIPinfoMessage(scip, nullptr, "\n");
	SCIP_CALL_EXC(SCIPpresolve(scip));

	//SCIPinfoMessage(scip, nullptr, "\nSolving...\n");
	SCIP_CALL_EXC(SCIPsolve(scip));

	SCIP_CALL_EXC(SCIPfreeTransform(scip));

	// Check if the solution contains any penalty variables
	std::vector<Agent*> penaltyAgents;

	if (SCIPgetNSols(scip) > 0)
	{
		SCIPinfoMessage(scip, nullptr, "\nSolution:\n");
		SCIP_CALL_EXC(SCIPprintSol(scip, SCIPgetBestSol(scip), nullptr, FALSE));

		SCIP_SOL* Solution = SCIPgetBestSol(scip);

		for (SCIP_VAR* var : allVariables)
		{
			if (!var)
			{
#if DEBUG_MIP
				std::cout << "VAR IS INVALID, SKIPPING IT!" << std::endl;
#endif

				continue;
			}

			double varSolution = SCIPgetSolVal(scip, Solution, var);

			// has chosen this path
			if (varSolution != 0)
			{
				std::cout << varNames[var] << " was chosen with value " << varSolution << std::endl;
				Agent* agent = varToAgentMap[var];

				bool isPathVariable = varToPathMap.find(var) != varToPathMap.end();
				if (isPathVariable)
				{
#if DEBUG_MIP
					std::cout << *agent << " FOUND A PATH SUCCESFULLY!" << std::endl;
#endif
					TemporalAStar::Path& path = *varToPathMap[var];
					agent->setPath(path);
				}
				else // the variable is a penalty var
				{
					agent->setPath(TemporalAStar::Path{ map->getTileAt(agent->x, agent->y) });

#if DEBUG_MIP
					std::cout << *agent << " was assigned the penalty var. We failed to find a solution!" << std::endl;
#endif

					penaltyAgents.push_back(agent);
				}
			}
			else
			{
#if DEBUG_MIP
				std::cout << varNames[var] << " was not chosen with value " << varSolution << std::endl;
#endif
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