#pragma once
#include "Tile.h"
#include <queue>
#include "GridMap.h"

struct Heuristic
{
public:
	Heuristic() = default;
	bool operator()(Tile* A, Tile* B)
	{
		return A->estimate < B->estimate;
	};
};


class AStar
{
private:
	GridMap* gridMap;
	std::vector<Tile*> modifiedTiles;

public:
	AStar(GridMap* gridMap);

public:
	using Path = std::deque<Tile*>;

private:
	using OpenQueue = std::vector<Tile*>;
	void AddNeighbor(OpenQueue& open, Tile* current, Tile* neighbor, Tile* start, Tile* goal);

public:
	Path FindPath(Tile* start, Tile* goal);
};

