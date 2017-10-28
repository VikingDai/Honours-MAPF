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
}

void PathAssigner::Init(std::vector<Agent*>& agents)
{
#if DEBUG_MIP
	std::cout << "Initializing Path Assignment Mixed Integer Problem" << std::endl;
#endif

	SCIP_CALL_EXC(SCIPcreate(&scip));
	SCIP_CALL_EXC(SCIPincludeDefaultPlugins(scip));

	/** create empty problem */
	SCIP_CALL_EXC(SCIPcreateProbBasic(scip, "PathAssignment"));

	/** disable output to console */
#if !DEBUG_MIP
	SCIP_CALL_EXC(SCIPsetIntParam(scip, "display/verblevel", 0));
#endif

	InitAgents(agents);
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
	pathToVarMap[pathRef] = pathVar;
	varToPathMap[pathVar] = pathRef;
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
	for (MAPF::AgentPathRef* pathRef : constraint)
	{
		SCIP_VAR* pathVar = pathToVarMap[pathRef];
		SCIP_CALL_EXC(SCIPaddCoefLinear(scip, collisionCons, pathVar, 1.0));
	}

	/** add constraint to list */
	pathCollisionCons.push_back(collisionCons);

	/** add then release constraint */
	SCIP_CALL_EXC(SCIPaddCons(scip, collisionCons));
	SCIP_CALL_EXC(SCIPreleaseCons(scip, &collisionCons));
}

void PathAssigner::Solve()
{
	mipTimer.Begin();
	std::cout << "Running MIP with " << allVariables.size() << " variables and " << pathCollisionCons.size() << " constraints" << std::endl;

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
				Agent* agent = varToPathMap[var]->agent;
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

	mipTimer.End();
	mipTimer.PrintTimeElapsed("MIP");
}

void PathAssigner::Cleanup()
{
	// release variables
	for (SCIP_VAR* var : allVariables)
		SCIP_CALL_EXC(SCIPreleaseVar(scip, &var));

	allVariables.clear();
	varToPathMap.clear();
	varNames.clear();

	agentChoiceConsMap.clear();
	pathCollisionCons.clear();
	agentPathVars.clear();


	SCIP_CALL_EXC(SCIPfree(&scip));

	for (MAPF::AgentPathRef* pathRef : usedPathRefs)
		MAPF::AgentPathRef::PATH_REF_POOL.push_back(pathRef);
	usedPathRefs.clear();
}
