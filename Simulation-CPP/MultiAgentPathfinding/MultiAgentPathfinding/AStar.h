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
		return A->estimate > B->estimate;
	};
};


class AStar
{
public:
	GridMap* gridMap;

public:
	AStar(GridMap* gridMap);

public:
	using OpenQueue = std::priority_queue<Tile*, std::vector<Tile*>, Heuristic>;
	using Path = std::queue<Tile*>;

	Path FindPath(Tile* start, Tile* goal);

	void AddNeighbor(OpenQueue& open, Tile* current, Tile* neighbour, Tile* start, Tile* goal);
};

