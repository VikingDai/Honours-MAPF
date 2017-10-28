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

class PathAssigner
{
public:
	PathAssigner(GridMap* map);

private:
	std::vector<MAPF::AgentPathRef*> usedPathRefs;

	GridMap* map;

	/** SCIP helper structures */
	std::vector<SCIP_VAR*> allVariables;
	std::map<SCIP_VAR*, MAPF::AgentPathRef*> varToPathMap;
	std::map<SCIP_VAR*, std::string> varNames;

	std::vector<SCIP_CONS*> pathCollisionCons;
	std::map<Agent*, SCIP_CONS*> agentChoiceConsMap;
	std::map<Agent*, std::vector<SCIP_VAR*>> agentPathVars;

private:
	SCIP* scip;

	/** Create penalty variable and agent choice constraint for each agent */
	void InitAgents(std::vector<Agent*> agents);

	std::map<MAPF::AgentPathRef*, SCIP_VAR*> pathToVarMap;
	SCIP_VAR* GetPathVar(MAPF::AgentPathRef* pathRef) {  return pathToVarMap[pathRef]; }

public:
	void Init(std::vector<Agent*>& agents);

	/** Assigns a path to each agent. Returns a vector of agents who were unable to find a conflict-free path. */
	std::vector<Agent*> AssignPaths(
		std::vector<Agent*>& agents, 
		std::vector<MAPF::PathConstraint>& constraints);

	int constraintCounter;

	void AddPath(MAPF::AgentPathRef* path);

	void AddConstraint(MAPF::PathConstraint& constraint);

	void Solve();

	void Cleanup();

private:
	Timer mipTimer;
};

