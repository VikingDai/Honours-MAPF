#pragma once

#include <vector>
#include "Agent.h"

/** SCIP includes */
#include <scip/scip.h>
#include <scip/scipexception.h>
#include <scip/scipdefplugins.h>
#include <scip/type_var.h>
#include <scip/struct_var.h>
#include "GridMap.h"

struct AgentMIP
{
	SCIP_CONS* choiceCons;
	std::vector<SCIP_VAR*> vars;
};

class PathAssigner
{
public:
	PathAssigner(GridMap* map);
	~PathAssigner();

private:
	using PathCollisions = std::vector<std::set<MAPF::AgentPathRef*>>;

	std::vector<MAPF::AgentPathRef*> usedPathRefs;

	GridMap* map;

	/** SCIP helper structures */
	std::vector<SCIP_VAR*> allVariables;
	std::map<SCIP_VAR*, Agent*> varToAgentMap;
	std::map<SCIP_VAR*, MAPF::AgentPathRef*> varToPathMap;
	std::map<SCIP_VAR*, std::string> varNames;

	std::vector<SCIP_CONS*> pathCollisionCons;
	std::map<Agent*, SCIP_CONS*> agentChoiceCons;
	std::map<Agent*, std::vector<SCIP_VAR*>> agentPathVars;

private:
	SCIP* scip;

	void Init();

	/** Create penalty variable and agent choice constraint for each agent */
	void InitAgent(std::vector<Agent*> agents);

	SCIP_RETCODE CreateProblem(
		std::vector<Agent*>& agents,
		PathCollisions& pathCollisions);

	/** Create a constraint describing that an agent can pick only one path or a penalty */
	void CreateAgentChoiceConstraints(int agentId, std::vector<SCIP_VAR*> agentVariables);

	/** Create constraints describing path collisions */
	void CreateCollisionConstraints(PathCollisions& pathCollisions);

	std::map<Agent*, std::map<int, SCIP_VAR*>> pathToVarMap;
	SCIP_VAR* GetPathVar(MAPF::AgentPathRef* path) { return pathToVarMap[path->agent][path->pathIndex]; }

public:
	/** Assigns a path to each agent. Returns a vector of agents who were unable to find a conflict-free path. */
	std::vector<Agent*> AssignPaths(
		std::vector<Agent*>& agents, 
		PathCollisions& collisions);

	void AddPath(Agent* agent, MAPF::AgentPathRef* path);

	void Cleanup();

private:
	Timer mipTimer;
};

