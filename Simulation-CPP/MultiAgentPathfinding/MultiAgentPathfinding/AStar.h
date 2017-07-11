#pragma once
#include <vector>
#include <queue>

class Tile;
class GridMap;

struct BaseHeuristic
{
	bool operator()(Tile* A, Tile* B);
};

typedef std::vector<Tile*> OpenQueue;

class AStar
{
private:
	GridMap* gridMap;
	std::vector<Tile*> visited;

public:
	AStar(GridMap* inGridMap);
	~AStar();

	std::deque<Tile*> findPath(Tile* start, Tile* goal);
	void AddToOpen(OpenQueue& open, Tile* from, Tile* tile, Tile* start, Tile* goal);
};

