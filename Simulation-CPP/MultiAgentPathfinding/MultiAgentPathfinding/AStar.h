#pragma once
#include <vector>
#include <queue>
#include <map>
#include "Timer.h"

class Tile;
class GridMap;

struct TileInfo
{
	int timestep;
	Tile* tile;
	float estimate;
	float cost;

	TileInfo(int timestep, Tile* tile, float cost, float estimate)
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
	BaseHeuristic(int timestep) { this->timestep = timestep; }

	bool operator()(TileInfo* A, TileInfo* B);
};

class AStar
{
public:
	using OpenQueue = std::vector<TileInfo*>;
	using Path = std::deque<Tile*>;
	using TileCosts = std::map<int, std::map<Tile*, float>>;

private:
	Timer timer;
	std::map<TileInfo*, TileInfo*> cameFrom;

	GridMap* gridMap;
	std::vector<Tile*> modifiedTiles;

public:
	AStar(GridMap* inGridMap);
	~AStar();

	Path FindPath(Tile* start, Tile* goal, TileCosts& customCostTable = TileCosts());
	void AddToOpen(OpenQueue& open, TileInfo* currentInfo, Tile* fromTile, Tile* tile, Tile* start, Tile* goal, TileCosts& customCosts);
};

