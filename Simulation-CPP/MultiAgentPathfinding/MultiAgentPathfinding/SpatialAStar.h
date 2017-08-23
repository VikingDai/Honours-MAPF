#pragma once
#include <vector>
#include <queue>
#include <map>
#include <list>
#include "Timer.h"
#include "Helper.h"

class Tile;
class GridMap;

struct TileTime
{
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

struct BaseHeuristic
{
private:
	int timestep;

public:
	BaseHeuristic() = default;
	bool operator()(TileTime* A, TileTime* B);
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
	
};

