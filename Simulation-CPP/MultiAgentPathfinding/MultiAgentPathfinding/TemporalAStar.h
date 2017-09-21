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
	AStarTileTime()
	{
		Reset();
	}

public:
	static std::vector<AStarTileTime*> TILE_TIME_POOL;
	static AStarTileTime* Make(std::vector<AStarTileTime*>& usedTileTimes);
	static void FillPool(int num)
	{
		for (int i = 0; i < num; i++)
			TILE_TIME_POOL.push_back(new AStarTileTime());
	};

	bool bClosed = false;
	bool bIsInOpen = false;

	AStarTileTime* parent;

	int timestep = 0;
	Tile* tile;
	float estimate = 0;
	float cost = 0;
	float customCost = 0;
	float heuristic = 0;

	void SetParent(AStarTileTime* parent)
	{
		this->parent = parent;
		this->cost = parent ? parent->cost + 1 : 0;
	}

	void UpdateCosts()
	{
		this->estimate = cost + customCost + heuristic;
	}

	void SetInfo(AStarTileTime* parent, int timestep, Tile* tile, float heuristic, float customCost)
	{
		SetParent(parent);

		this->timestep = timestep;
		this->tile = tile;

		this->heuristic = heuristic;
		this->customCost = customCost;

		UpdateCosts();
	}

	void Reset()
	{
		bClosed = false;
		parent = nullptr;
		bIsInOpen = false;

		parent = nullptr;
		tile = nullptr;
		timestep = estimate = cost = customCost = heuristic = 0;
	}
};

struct BaseHeuristic
{
public:
	BaseHeuristic() = default;
	bool operator()(AStarTileTime* A, AStarTileTime* B);
	friend bool operator==(const AStarTileTime &a, const AStarTileTime &b)
	{
		return a.timestep == b.timestep && a.tile == b.tile;
	}
};

class TemporalAStar
{
private:
	std::vector<AStarTileTime*> usedTileTimes;
	std::vector<AStarTileTime*> closed;
	bool needsSorting;

public:


	static int GLOBAL_TILES_EXPANDED;
	int LOCAL_TILES_EXPANDED;

public:
	using TileCosts = std::map<int, std::map<Tile*, float>>;

private:
	Timer timer;
	Timer sortTimer;

	GridMap* gridMap;

public:
	TemporalAStar(GridMap* gridMap);
	void SetGridMap(GridMap* gridMap) { this->gridMap = gridMap; }
	~TemporalAStar();

public:
	using OpenQueue = std::vector<AStarTileTime*>;//std::priority_queue<AStarTileTime*, std::vector<AStarTileTime*>, BaseHeuristic>;

	std::set<Tile*> modifiedTiles;
	std::set<AStarTileTime*> modifiedTileTimes;

	std::map<Tile*, std::map<int, AStarTileTime*>> spatialGridMap;

	MAPF::Path FindPath(Tile* start, Tile* goal, TileCosts& customCosts = TileCosts());

	void ExpandNeighbor(OpenQueue& open, AStarTileTime* current, Tile* neighborTile, Tile* start, Tile* goal, TileCosts& customCosts);

	int GetCustomCosts(int timestep, Tile* tile, TileCosts& customCosts);
};

