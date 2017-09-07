#pragma once

#include <set>
#include <stack>
#include <queue>
#include "Tile.h"
#include "GridMap.h"
#include "MAPF.h"

class TemporalBFS
{
public:
	int nodesExpanded;

public:
	GridMap* gridMap;

public:
	using TileTime = std::pair<Tile*, int>;

	std::queue<TileTime*> frontier;
	std::vector<MAPF::Path> allPaths;

	std::map<TileTime*, TileTime*> cameFrom;

public:
	TemporalBFS(GridMap* gridMap);

public:
	MAPF::Path FindNextPath(Tile* start, Tile* goal);
	std::vector<MAPF::Path> SearchToDepth(Tile* start, Tile* goal, int depth);
	void AddNeighbor(TileTime* current, Tile* neighbor);
};

