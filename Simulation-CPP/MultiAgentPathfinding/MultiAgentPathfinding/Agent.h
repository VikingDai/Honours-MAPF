#pragma once
#include "EObject.h"
#include <vector>
#include <deque>
#include <iostream>
#include "AStar.h"

class Tile;
class GridMap;

class Agent : public EObject
{
private:
	AStar* search;
	GridMap* map;

	int agentId;

public:
	vec3 color;
	Agent(int x, int y);
	virtual void step();
	void setPath(AStar::Path& path);

	std::vector<AStar::Path> allPaths;
	AStar::Path path;

	int getAgentId() { return agentId; }

	vec3 renderPos;
	void update(float dt);

	friend std::ostream& operator<<(std::ostream& os, Agent& agent);
};

