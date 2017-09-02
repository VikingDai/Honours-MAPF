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

private: /** SCIP variables */
	SCIP* scip;

	void InitSCIP();

	using PathCollisions = std::vector<std::set<TemporalAStar::Path*>>;

	SCIP_RETCODE CreateProblem(
		std::vector<Agent*>& agents,
		PathCollisions& pathCollisions,
		std::map<TemporalAStar::Path*, int>& pathLengths);

public:
	/** Assigns a path to each agent. Returns a vector of agents who were unable to find a conflict-free path. */
	std::vector<Agent*> AssignPaths(
		std::vector<Agent*> agents, 
		PathCollisions& collisions,
		std::map<TemporalAStar::Path*, int>& pathLengths);

	void Cleanup();
};

