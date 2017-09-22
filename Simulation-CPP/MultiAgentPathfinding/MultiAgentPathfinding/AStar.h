#pragma once
#include "Tile.h"
#include <queue>
#include "GridMap.h"
#include "Timer.h"

struct Heuristic
{
public:
	Heuristic() = default;
	bool operator()(Tile* A, Tile* B)
	{
		if (A->estimate == B->estimate)
			return A->cost < B->cost;

		return A->estimate > B->estimate;
	};
};


class AStar
{
private:

	int LOCAL_EXP;
	int LOCAL_GEN;
	int LOCAL_TOUCH;
	GridMap* gridMap;
	std::vector<Tile*> modifiedTiles;

	Timer timer;

public:
	AStar(GridMap* gridMap);

public:
	using Path = std::deque<Tile*>;

private:
	using OpenQueue = std::vector<Tile*>;
	//using OpenQueue = std::priority_queue<Tile*, std::vector<Tile*>, Heuristic>;
	void AddNeighbor(OpenQueue& open, Tile* current, Tile* neighbor, Tile* start, Tile* goal);

public:
	Path FindPath(Tile* start, Tile* goal);
};

