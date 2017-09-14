#pragma once

#include <set>
#include <stack>
#include <queue>
#include "Tile.h"
#include "GridMap.h"
#include "MAPF.h"

class TemporalBFS
{
	using BFSTileTime = std::pair<Tile*, int>;

	static std::vector<BFSTileTime*> TILE_TIME_POOL;
	static BFSTileTime* MakeTileTime(std::vector<BFSTileTime*>& usedTileTimes, Tile* tile, int time);
	std::vector<BFSTileTime*> usedTileTimes;

public:
	int nodesExpanded;

public:
	GridMap* gridMap;

public:

	std::queue<BFSTileTime*> frontier;
	std::vector<MAPF::Path> allPaths;

	std::map<BFSTileTime*, BFSTileTime*> cameFrom;

public:
	TemporalBFS(GridMap* gridMap);

	void Reset();

public:
	MAPF::Path FindNextPath(Tile* start, Tile* goal);
	void AddNeighbor(BFSTileTime* current, Tile* neighbor);

	std::vector<MAPF::Path> SearchToDepth(Tile* start, Tile* goal, int depth);
};

