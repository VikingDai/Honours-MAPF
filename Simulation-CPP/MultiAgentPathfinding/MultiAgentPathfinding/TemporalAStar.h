#pragma once
#include <vector>
#include <queue>
#include <map>
#include <list>
#include "Timer.h"
#include "Helper.h"
#include <set>
#include "MAPF.h"

class Tile;
class GridMap;

struct TileTime
{
	bool bNeedsReset;

	bool bClosed = false;
	bool bIsInOpen = false;

	TileTime* parent;

	int timestep;
	std::map<TileTime*, int> countFrom;
	Tile* tile;
	float estimate;
	float cost;

	void SetInfo(int timestep, Tile* tile, float cost, float estimate)
	{
		this->timestep = timestep;
		this->tile = tile;
		this->estimate = estimate;
		this->cost = cost;
	}

	void SetParent(TileTime* parent)
	{
		this->parent = parent;
	}

	void Reset()
	{
		bNeedsReset = true;
		bClosed = false;
		bIsInOpen = false;
	}
};

struct BaseHeuristic
{
public:
	BaseHeuristic() = default;
	bool operator()(TileTime* A, TileTime* B);
};

class TemporalAStar
{
public:
	static int GLOBAL_TILES_EXPANDED;
	int LOCAL_TILES_EXPANDED;

public:
	using TileCosts = std::map<int, std::map<Tile*, float>>;

private:
	Timer timer;
	Timer sortTimer;

	GridMap* gridMap;
	std::vector<Tile*> modifiedTiles;

public:
	TemporalAStar(GridMap* gridMap);
	void SetGridMap(GridMap* gridMap) { this->gridMap = gridMap; }
	~TemporalAStar();

public:
	using OpenQueue = std::priority_queue<TileTime*, std::vector<TileTime*>, BaseHeuristic>;

	std::set<TileTime*> modifiedTileTimes;

	std::map<Tile*, std::map<int, TileTime*>> spatialGridMap;

	MAPF::Path FindPath(Tile* start, Tile* goal, TileCosts& customCosts = TileCosts());

	void ExpandNeighbor(OpenQueue& open, TileTime* current, Tile* neighborTile, Tile* start, Tile* goal, TileCosts& customCosts);

	int GetCustomCosts(int timestep, Tile* tile, TileCosts& customCosts);
};

