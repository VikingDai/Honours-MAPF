#pragma once
#include "GridMap.h"
#include "Agent.h"
#include "Environment.h"

class Scenario
{
public:
	GridMap* gridMap;
	std::vector<Agent*> agents;

	Scenario() = default;

	std::vector<int> agentInfo;

	void LoadScenario(std::string filename, Environment& environment);
};

