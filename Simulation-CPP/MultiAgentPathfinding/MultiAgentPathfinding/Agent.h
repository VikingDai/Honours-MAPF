#pragma once
#include "EObject.h"
#include <vector>

class AStar;
class Tile;
class GridMap;

class Agent : public EObject
{
private:
	
	AStar* search;
	GridMap* map;

public:
	std::vector<Tile*> path;
	Agent(GridMap* map, AStar* search, int x, int y);
	virtual void Step();
};

