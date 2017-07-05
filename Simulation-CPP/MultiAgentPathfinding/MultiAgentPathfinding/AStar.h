#pragma once
#include <vector>
#include <queue>

class Tile;
class GridMap;

class AStar
{
private:
	GridMap* gridMap;
	std::vector<Tile*> closed;

public:
	AStar(GridMap* inGridMap);
	~AStar();

	std::vector<Tile*> findPath(Tile* start, Tile* goal);
	void AddToOpen(std::priority_queue<Tile*>& open, Tile* tile, int x, int y);
};

