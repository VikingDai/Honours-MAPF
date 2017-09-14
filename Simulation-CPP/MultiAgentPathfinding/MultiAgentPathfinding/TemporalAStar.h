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

struct AStarTileTime
{
private:
	AStarTileTime() = default;

public:
	static std::vector<AStarTileTime*> TILE_TIME_POOL;
	static AStarTileTime* Make(std::vector<AStarTileTime*>& usedTileTimes);

	bool bNeedsReset;

	bool bClosed = false;
	bool bIsInOpen = false;

	AStarTileTime* parent;

	int timestep;
	std::map<AStarTileTime*, int> countFrom;
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

	void SetParent(AStarTileTime* parent)
	{
		this->parent = parent;
	}

	void Reset()
	{
		bNeedsReset = true;
		bClosed = false;
		parent = nullptr;
		bIsInOpen = false;
		countFrom.clear();
	}
};

struct BaseHeuristic
{
public:
	BaseHeuristic() = default;
	bool operator()(AStarTileTime* A, AStarTileTime* B);
};

class TemporalAStar
{
private:
	std::vector<AStarTileTime*> usedTileTimes;

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
	using OpenQueue = std::priority_queue<AStarTileTime*, std::vector<AStarTileTime*>, BaseHeuristic>;

	std::set<AStarTileTime*> modifiedTileTimes;

	std::map<Tile*, std::map<int, AStarTileTime*>> spatialGridMap;

	MAPF::Path FindPath(Tile* start, Tile* goal, TileCosts& customCosts = TileCosts());

	void ExpandNeighbor(OpenQueue& open, AStarTileTime* current, Tile* neighborTile, Tile* start, Tile* goal, TileCosts& customCosts);

	int GetCustomCosts(int timestep, Tile* tile, TileCosts& customCosts);
};

