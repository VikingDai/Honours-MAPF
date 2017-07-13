#pragma once
#include "EObject.h"
#include <vector>
#include <deque>

class AStar;
class Tile;
class GridMap;

class Agent : public EObject
{
private:
	AStar* search;
	GridMap* map;
	int pathIndex;
	
	int agentId;

public:
	vec3 color;
	Agent(int x, int y);
	virtual void step();
	void setPath(std::deque<Tile*> path);
	//std::deque<Tile*>& getPath() { return path; }

	std::deque<Tile*> path;
	bool hasReachedGoal;

	int getAgentId() { return agentId; }
};

