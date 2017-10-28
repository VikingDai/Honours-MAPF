#include "PathAssigner.h"

#include "Statistics.h"
#include <sstream>
#include <string>

#define DEBUG_MIP 0
#define DEBUG_STATS 0

PathAssigner::PathAssigner(GridMap* inGridMap)
	: constraintCounter(0)
{
	map = inGridMap;
	Init();
}

PathAssigner::~PathAssigner()
{
}

void PathAssigner::Init()
{
#if DEBUG_MIP
	std::cout << "Initializing Path Assignment Mixed Integer Problem" << std::endl;
#endif

	SCIP_CALL_EXC(SCIPcreate(&scip));
	SCIP_CALL_EXC(SCIPincludeDefaultPlugins(scip));

	// create empty problem
	SCIP_CALL_EXC(SCIPcreateProbBasic(scip, "PathAssignment"));

	// disable output to console

#if !DEBUG_MIP
	SCIP_CALL_EXC(SCIPsetIntParam(scip, "display/verblevel", 0));
#endif
}

void PathAssigner::InitAgents(std::vector<Agent*> agents)
{
	for (Agent* agent : agents)
	{
		/** create penalty variable */
		SCIP_VAR* penaltyVar;
		std::ostringstream penaltyVarNameStream;
		penaltyVarNameStream << "a" << agent->GetAgentId() << "q";
		const char* penaltyVarName = penaltyVarNameStream.str().c_str();
		char penaltyVarNameC[50];
		sprintf(penaltyVarNameC, "a%dq", agent->GetAgentId());

		SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &penaltyVar, penaltyVarNameC, 0, 1, 1000, SCIP_VARTYPE_INTEGER));
		SCIP_CALL_EXC(SCIPaddVar(scip, penaltyVar));

		varToAgentMap[penaltyVar] = agent;
		varNames[penaltyVar] = penaltyVarNameStream.str();

		allVariables.push_back(penaltyVar);

		/** create choice constraint */
		SCIP_CONS* agentChoiceCons;
		char choiceConsName[50];
		sprintf_s(choiceConsName, "agentChoice%d", agent->GetAgentId());

		SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &agentChoiceCons, choiceConsName, 0, nullptr, nullptr, 1, 1));

		/** add penalty var to constraint */
		SCIP_CALL_EXC(SCIPaddCoefLinear(scip, agentChoiceCons, penaltyVar, 1.0));

		SCIP_CALL_EXC(SCIPaddCons(scip, agentChoiceCons)); // add constraint

		agentChoiceConsMap[agent] = agentChoiceCons;
	}
}

SCIP_RETCODE PathAssigner::CreateProblem(std::vector<Agent*>& agents, std::vector<MAPF::PathConstraint>& constraints)
{
	// create empty problem
	const SCIP_Real NEG_INFINITY = -SCIPinfinity(scip);
	const SCIP_Real POS_INFINITY = SCIPinfinity(scip);

	// create variables for paths as well as penalties and constraints
	pathToVarMap.clear();

	for (Agent* agent : agents)
	{
		std::vector<SCIP_VAR*> agentVariables;

		// get the agent's id
		int agentId = agent->GetAgentId();

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

		// for each path construct a variable in the form 'a1p1'
		std::vector<MAPF::Path>& paths = agent->pathBank;
		for (int i = 0; i < paths.size(); i++)
		{
			MAPF::AgentPathRef* path = new MAPF::AgentPathRef(agent, i); // MAPF::AgentPathRef::Make(agent, i, usedPathRefs);
			assert(!path->GetPath().tiles.empty());

			// create variable describing path
			SCIP_VAR* pathVar;

			std::ostringstream pathVarNameStream;
			pathVarNameStream << "a" << agentId << "p" << i;
			const char* pathVarName = pathVarNameStream.str().c_str();

			char pathVarNameC[50];
			sprintf(pathVarNameC, "a%dp%d", agentId, i);

			SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &pathVar, pathVarNameC, 0, 1, path->GetPath().cost, SCIP_VARTYPE_INTEGER));
			SCIP_CALL_EXC(SCIPaddVar(scip, pathVar));

			// add it to the map
			pathToVarMap[path->agent][path->pathIndex] = pathVar;
			varToPathMap[pathVar] = path;
			varToAgentMap[pathVar] = agent;
			varNames[pathVar] = pathVarNameStream.str();

			allVariables.push_back(pathVar);
			agentVariables.push_back(pathVar);
		}

		CreateAgentChoiceConstraints(agentId, agentVariables);
	}

	CreateCollisionConstraints(constraints);


	return SCIP_OKAY;
}

void PathAssigner::AddPath(MAPF::AgentPathRef* pathRef)
{
	/** create path variable name */
	int pathId = agentPathVars[pathRef->agent].size();
	std::ostringstream pathVarNameStream;
	pathVarNameStream << "a" << pathRef->agent->GetAgentId() << "p" << pathId;
	const char* pathVarName = pathVarNameStream.str().c_str();

	char pathVarNameC[50];
	sprintf(pathVarNameC, "a%dp%d", pathRef->agent->GetAgentId(), pathId);


	/** create variable describing path */
	SCIP_VAR* pathVar;
	int pathCost = pathRef->GetPath().cost;
	SCIP_CALL_EXC(SCIPcreateVarBasic(scip, &pathVar, pathVarNameC, 0, 1, pathCost, SCIP_VARTYPE_INTEGER));
	SCIP_CALL_EXC(SCIPaddVar(scip, pathVar));

	/** add path to agent choice constraint */
	SCIP_CONS* agentChoiceCons = agentChoiceConsMap[pathRef->agent];
	SCIP_CALL_EXC(SCIPaddCoefLinear(scip, agentChoiceCons, pathVar, 1.0));

	/** add it to the map */
	pathToVarMap[pathRef->agent][pathRef->pathIndex] = pathVar;
	varToPathMap[pathVar] = pathRef;
	varToAgentMap[pathVar] = pathRef->agent;
	varNames[pathVar] = pathVarNameStream.str();

	allVariables.push_back(pathVar);
	agentPathVars[pathRef->agent].push_back(pathVar);
}

void PathAssigner::AddConstraint(MAPF::PathConstraint& constraint)
{
	/** make constraint name */
	char collisionConsName[50];
	sprintf_s(collisionConsName, "collision%d", constraintCounter);
	constraintCounter += 1;

	/** create the constraint */
	SCIP_CONS* collisionCons;
	SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &collisionCons, collisionConsName, 0, nullptr, nullptr, 0, 1));

	/** apply collision constraints to path variables */
	for (MAPF::AgentPathRef* path : constraint)
	{
		SCIP_VAR* pathVar = pathToVarMap[path->agent][path->pathIndex];
		SCIP_CALL_EXC(SCIPaddCoefLinear(scip, collisionCons, pathVar, 1.0));
	}

	/** add then release constraint */
	SCIP_CALL_EXC(SCIPaddCons(scip, collisionCons));
	SCIP_CALL_EXC(SCIPreleaseCons(scip, &collisionCons));
}

void PathAssigner::Solve()
{
	SCIP_CALL_EXC(SCIPpresolve(scip));

	SCIP_CALL_EXC(SCIPsolve(scip));

	SCIP_CALL_EXC(SCIPfreeTransform(scip));

	if (SCIPgetNSols(scip) > 0)
	{
		SCIP_SOL* solution = SCIPgetBestSol(scip);

		for (SCIP_VAR* var : allVariables)
		{
			double varSolution = SCIPgetSolVal(scip, solution, var);

			if (varSolution > 0) // has chosen this path
			{
				Agent* agent = varToAgentMap[var];
				bool isPathVariable = varToPathMap.find(var) != varToPathMap.end();
				if (isPathVariable)
				{
					agent->SetPath(varToPathMap[var]);
				}
				else // the variable is a penalty var
				{
					agent->SetPath(nullptr);
				}
			}
		}
	}
	else
	{
		assert(false);
	}
}

void PathAssigner::CreateAgentChoiceConstraints(int agentId, std::vector<SCIP_VAR*> agentVariables)
{
	SCIP_CONS* agentChoiceCons;
	char choiceConsName[50];
	sprintf_s(choiceConsName, "agentChoice%d", agentId);

	SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &agentChoiceCons, choiceConsName, 0, nullptr, nullptr, 1, 1));

	for (SCIP_VAR* agentPath : agentVariables)
		SCIP_CALL_EXC(SCIPaddCoefLinear(scip, agentChoiceCons, agentPath, 1.0));

	SCIP_CALL_EXC(SCIPaddCons(scip, agentChoiceCons)); // add constraint
	SCIP_CALL_EXC(SCIPreleaseCons(scip, &agentChoiceCons)); // release it
}

void PathAssigner::CreateCollisionConstraints(std::vector<MAPF::PathConstraint>& constraints)
{
	for (int i = 0; i < constraints.size(); i++)
	{
		std::set<MAPF::AgentPathRef*>& paths = constraints[i];

		SCIP_CONS* collisionCons;
		char collisionConsName[50];
		sprintf_s(collisionConsName, "collision%d", i);
		SCIP_CALL_EXC(SCIPcreateConsBasicLinear(scip, &collisionCons, collisionConsName, 0, nullptr, nullptr, 0, 1));

		// apply collision constraints to path variables
		for (MAPF::AgentPathRef* path : paths)
		{
			SCIP_VAR* var = pathToVarMap[path->agent][path->pathIndex];
			SCIP_CALL_EXC(SCIPaddCoefLinear(scip, collisionCons, var, 1.0));
		}

		// add then release constraint
		SCIP_CALL_EXC(SCIPaddCons(scip, collisionCons));
		SCIP_CALL_EXC(SCIPreleaseCons(scip, &collisionCons));
	}
}

std::vector<Agent*> PathAssigner::AssignPaths(
	std::vector<Agent*>& agents,
	std::vector<MAPF::PathConstraint>& constraints)
{
	mipTimer.Begin();

	Init();

	SCIP_CALL_EXC(CreateProblem(agents, constraints));

#if DEBUG_MIP
	SCIPinfoMessage(scip, nullptr, "Original problem:\n");
	SCIP_CALL_EXC(SCIPprintOrigProblem(scip, nullptr, "cip", FALSE));
	SCIPinfoMessage(scip, nullptr, "\n");
#endif

	SCIP_CALL_EXC(SCIPpresolve(scip));

#if DEBUG_MIP
	SCIPinfoMessage(scip, nullptr, "\nSolving...\n");
#endif

	SCIP_CALL_EXC(SCIPsolve(scip));

	SCIP_CALL_EXC(SCIPfreeTransform(scip));

	// Check if the solution contains any penalty variables
	std::vector<Agent*> penaltyAgents;

	if (SCIPgetNSols(scip) > 0)
	{
#if DEBUG_MIP
		SCIPinfoMessage(scip, nullptr, "\nSolution:\n");
		SCIP_CALL_EXC(SCIPprintSol(scip, SCIPgetBestSol(scip), nullptr, FALSE));
#endif

		SCIP_SOL* Solution = SCIPgetBestSol(scip);

		for (SCIP_VAR* var : allVariables)
		{
			assert(var); // this should not happen

			double varSolution = SCIPgetSolVal(scip, Solution, var);

			// has chosen this path
			if (varSolution != 0)
			{
#if DEBUG_MIP
				//std::cout << varNames[var] << " was chosen with value " << varSolution << std::endl;
#endif
				Agent* agent = varToAgentMap[var];

				bool isPathVariable = varToPathMap.find(var) != varToPathMap.end();
				if (isPathVariable)
				{
#if DEBUG_MIP
					//std::cout << *agent << " FOUND A PATH SUCCESFULLY!" << std::endl;
#endif
					agent->SetPath(varToPathMap[var]);
				}
				else // the variable is a penalty var
				{
					//agent->setPath(MAPF::Path{ map->getTileAt(agent->x, agent->y) });
#if DEBUG_MIP
					//std::cout << *agent << " was assigned the penalty var. We failed to find a solution!" << std::endl;
#endif
					agent->SetPath(nullptr);
					penaltyAgents.push_back(agent);
				}
			}
			else
			{
#if DEBUG_MIP
				//std::cout << varNames[var] << " was not chosen with value " << varSolution << std::endl;
#endif
			}
		}
	}
	else
	{
#if DEBUG_MIP
		std::cout << "ERROR: SCIP failed to find any solutions" << std::endl;
#endif
	}

	Cleanup();

	mipTimer.End();
#if DEBUG_STATS
	mipTimer.PrintTimeElapsed("MIP Path assignment");
#endif
	Stats::avgMipTime = mipTimer.GetAvgTime();

	return penaltyAgents;
}

void PathAssigner::Cleanup()
{
	// release variables
	for (SCIP_VAR* var : allVariables)
		SCIP_CALL_EXC(SCIPreleaseVar(scip, &var));

	allVariables.clear();
	varToAgentMap.clear();
	varToPathMap.clear();
	varNames.clear();

	SCIP_CALL_EXC(SCIPfree(&scip));

	for (MAPF::AgentPathRef* pathRef : usedPathRefs)
		MAPF::AgentPathRef::PATH_REF_POOL.push_back(pathRef);
	usedPathRefs.clear();
}
