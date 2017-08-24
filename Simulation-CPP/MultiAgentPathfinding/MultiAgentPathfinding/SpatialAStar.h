#pragma once
#include <vector>
#include <queue>
#include <map>
#include <list>
#include "Timer.h"
#include "Helper.h"
#include <set>

class Tile;
class GridMap;

struct TileTime
{
	int timesUsed;
	int timestep;
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
};

struct TileTime2
{
	bool bNeedsReset;

	bool bClosed = false;
	bool bIsInOpen = false;

	TileTime2* parent;

	int timestep;
	std::map<TileTime2*, int> countFrom;
	int timesUsed = 0;
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

	void SetParent(TileTime2* parent)
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
private:
	int timestep;

public:
	BaseHeuristic() = default;
	bool operator()(TileTime* A, TileTime* B);
};

struct BaseHeuristic2
{
private:
	int timestep;

public:
	BaseHeuristic2() = default;
	bool operator()(TileTime2* A, TileTime2* B);
};

class SpatialAStar
{
public:
	std::list<TileTime*> freeTileInfos;
	std::list<TileTime*> usedTileInfos;

public:
	std::map<TileTime*, Helper::Action> agentActionMap;

public:
	using OpenQueue = std::priority_queue<TileTime*, std::vector<TileTime*>, BaseHeuristic>;
	using Path = std::deque<Tile*>;
	using TileCosts = std::map<int, std::map<Tile*, float>>;

private:
	Timer timer;
	Timer sortTimer;

	BaseHeuristic heuristic;
	std::map<TileTime*, TileTime*> cameFrom;

	GridMap* gridMap;
	std::vector<Tile*> modifiedTiles;

public:
	SpatialAStar(GridMap* gridMap);
	void SetGridMap(GridMap* gridMap) { this->gridMap = gridMap; }
	~SpatialAStar();

	Path FindPath(Tile* start, Tile* goal, TileCosts& customCostTable = TileCosts());
	void ExpandNeighbor(OpenQueue& open, TileTime* currentInfo, Tile* currentTile, Tile* neighborTile, Tile* start, Tile* goal, TileCosts& customCosts);


	std::map<Tile*, std::map<int, int>> visitedAtTimeCount;


public: // TESTING NEW STUFF
	using OpenQueue2 = std::priority_queue<TileTime2*, std::vector<TileTime2*>, BaseHeuristic2>;

	std::set<TileTime2*> modifiedTileTimes;

	std::map<Tile*, std::map<int, TileTime2*>> spatialGridMap;

	Path FindPath2(Tile* start, Tile* goal);

	//OpenQueue2 open2;

	void ExpandNeighbor2(OpenQueue2& open, TileTime2* current, Tile* neighborTile, Tile* start, Tile* goal);
};

