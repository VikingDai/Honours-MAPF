#pragma once
#include <vector>
#include <queue>
#include <map>

class Tile;
class GridMap;

struct BaseHeuristic
{
	bool operator()(Tile* A, Tile* B);
};



class AStar
{
public:
	using OpenQueue = std::vector<Tile*>;
	using Path = std::deque<Tile*>;
	using TileCosts = std::map<Tile*, float>;

private:
	GridMap* gridMap;
	std::vector<Tile*> visited;

public:
	AStar(GridMap* inGridMap);
	~AStar();

	Path findPath(Tile* start, Tile* goal, TileCosts& customCosts = TileCosts());
	void AddToOpen(OpenQueue& open, Tile* from, Tile* tile, Tile* start, Tile* goal, TileCosts& customCosts);
};

