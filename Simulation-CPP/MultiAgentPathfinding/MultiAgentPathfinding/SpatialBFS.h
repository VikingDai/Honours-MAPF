#pragma once

#include <set>
#include <stack>
#include <queue>
#include "Tile.h"
#include "GridMap.h"

class SpatialBFS
{
public:
	int nodesExpanded;

public:
	GridMap* gridMap;

public:
	using TileTime = std::pair<Tile*, int>;

	using Path = std::deque<Tile*>;

	std::queue<TileTime*> frontier;
	std::vector<Path> allPaths;

	std::map<TileTime*, TileTime*> cameFrom;

public:
	SpatialBFS(GridMap* gridMap);

public:
	Path FindNextPath(Tile* start, Tile* goal);
	std::vector<Path> SearchToDepth(Tile* start, Tile* goal, int depth);
	void AddNeighbor(TileTime* current, Tile* neighbor);
};

