#pragma once

#include <vector>
#include "Agent.h"
#include "TemporalAStar.h"

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
	~PathAssigner();

private:
	GridMap* map;

	/** SCIP helper structures */
	std::vector<SCIP_VAR*> allVariables;
	std::map<SCIP_VAR*, Agent*> varToAgentMap;
	std::map<SCIP_VAR*, TemporalAStar::Path*> varToPathMap;
	std::map<SCIP_VAR*, std::string> varNames;

public:
	using PathCollisions = std::vector<std::set<TemporalAStar::Path*>>;

	SCIP_RETCODE SetupProblem(
		SCIP* scip, 
		std::vector<Agent*>& agents, 
		PathCollisions& pathCollisions, 
		std::map<TemporalAStar::Path*, int>& PathLengths);

	/** Assigns a path to each agent. Returns a vector of agents who were unable to find a conflict-free path. */
	std::vector<Agent*> AssignPaths(
		std::vector<Agent*> agents, 
		PathCollisions& collisions,
		std::map<TemporalAStar::Path*, int>& PathLengths);
};

