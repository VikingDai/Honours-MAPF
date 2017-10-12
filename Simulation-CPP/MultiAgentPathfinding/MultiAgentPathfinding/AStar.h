#pragma once
#include "Tile.h"
#include <queue>
#include "GridMap.h"
#include "Timer.h"
#include "PriorityQueue.h"

struct Heuristic
{
public:
	Heuristic() = default;
	bool operator()(Tile* A, Tile* B)
	{
		if (A->f == B->f)
			return A->g < B->g;

		return A->f > B->f;
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
	using OpenQueue = PriorityQueue<Tile*>;
	void AddNeighbor(OpenQueue& open, Tile* current, Tile* neighbor, Tile* start, Tile* goal);

public:
	Path FindPath(Tile* start, Tile* goal);
};

