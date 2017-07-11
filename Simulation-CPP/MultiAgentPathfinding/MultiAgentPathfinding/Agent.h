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
	std::deque<Tile*> path;

public:
	vec3 color;
	Agent(int x, int y);
	virtual void step();
	void setPath(std::deque<Tile*> path);
	std::deque<Tile*> getPath() { return path; }
	bool hasReachedGoal;
};

