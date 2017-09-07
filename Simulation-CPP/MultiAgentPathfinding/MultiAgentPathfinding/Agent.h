#pragma once
#include "EObject.h"
#include <vector>
#include <deque>
#include <iostream>
#include "TemporalAStar.h"
#include "Graphics.h"
#include "TemporalBFS.h"
#include "AStar.h"
#include "MAPF.h"

class Tile;
class GridMap;

class Agent : public EObject
{
public:
	Agent(GridMap* gridMap, Tile* startTile, Tile* goalTile = nullptr);

private:
	static int agentCounter;
public:
	static void ResetAgentCounter() { agentCounter = 0; }

public:
	TemporalBFS* bfs;
	TemporalAStar* temporalAStar;
	AStar* aStar;

private:
	int agentId;

	MAPF::Path chosenPath;
public:
	std::vector<MAPF::Path> potentialPaths;

	void setPath(MAPF::Path& path);
	MAPF::Path& getPath() { return chosenPath; }

public:
	Tile* goal;

	vec3 color;
	
	virtual void step();
	

	std::map<Tile*, int> customWeights;

	int getAgentId() { return agentId; }

	vec3 renderPos;
	void update(float dt);

	void drawPaths(Graphics* graphics);
	void drawLineToGoal(Graphics* graphics);

	friend std::ostream& operator<<(std::ostream& os, Agent& agent);
};

struct AgentPathRef
{
	Agent* agent;
	int pathIndex;

	AgentPathRef(Agent* agent, int pathIndex) : agent(agent), pathIndex(pathIndex) {}

	MAPF::Path& getPath() { return agent->potentialPaths[pathIndex]; }

	friend std::ostream& operator<<(std::ostream& os, AgentPathRef& pathRef)
	{
		os << "Path " << pathRef.pathIndex << ": Agent(" << pathRef.agent->getAgentId() << ")";
		return os;
	}
};