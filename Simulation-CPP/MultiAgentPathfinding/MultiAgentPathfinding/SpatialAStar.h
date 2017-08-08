#pragma once
#include <vector>
#include <queue>
#include <map>
#include <list>
#include "Timer.h"
#include "Helper.h"

class Tile;
class GridMap;

struct TileInfo
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
	bool operator()(TileInfo* A, TileInfo* B);
};

class SpatialAStar
{
public:
	std::list<TileInfo*> freeTileInfos;
	std::list<TileInfo*> usedTileInfos;

public:
	std::map<TileInfo*, Helper::Action> agentActionMap;

public:
	using OpenQueue = std::priority_queue<TileInfo*, std::vector<TileInfo*>, BaseHeuristic>;
	using Path = std::deque<Tile*>;
	using TileCosts = std::map<int, std::map<Tile*, float>>;

private:
	Timer timer;
	Timer sortTimer;

	BaseHeuristic heuristic;
	std::map<TileInfo*, TileInfo*> cameFrom;

	GridMap* gridMap;
	std::vector<Tile*> modifiedTiles;

public:
	SpatialAStar(GridMap* gridMap);
	void SetGridMap(GridMap* gridMap) { this->gridMap = gridMap; }
	~SpatialAStar();

	Path FindPath(Tile* start, Tile* goal, TileCosts& customCostTable = TileCosts());
	void AddToOpen(OpenQueue& open, TileInfo* currentInfo, Tile* fromTile, Tile* tile, Tile* start, Tile* goal, TileCosts& customCosts);
};

