#pragma once

#include "Environment.h"

/* Describes the configuration of the environment: grid map, agents and obstacles. 
Reads .scenario files and load these parameters from them. */
class Scenario
{
private:
	using Pos = std::pair<int, int>;
	using AgentInfo = std::pair<Pos, Pos>;

	int numRandomAgents;
	

public:
	Scenario() = default;
	void LoadFromFile(std::string filename, Environment& environment);
};

