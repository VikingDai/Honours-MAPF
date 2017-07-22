#pragma once
#include <vector>
#include <queue>
#include <map>

class Tile;
class GridMap;

struct TileInfo
{
	float cost;
	std::vector<Tile*> path;
	Tile* tile;
};

struct BaseHeuristic
{
	bool operator()(Tile* A, Tile* B);
};

class AStar
{
public:
	using OpenQueue = std::vector<Tile*>;
	using Path = std::deque<Tile*>;
	using TileCosts = std::map<int, std::map<Tile*, float>>;

private:
	GridMap* gridMap;
	std::vector<Tile*> visited;

public:
	AStar(GridMap* inGridMap);
	~AStar();

	Path findPath(Tile* start, Tile* goal, TileCosts& customCostTable = TileCosts());
	void AddToOpen(OpenQueue& open, Tile* from, Tile* tile, Tile* start, Tile* goal, TileCosts& customCosts);
};

