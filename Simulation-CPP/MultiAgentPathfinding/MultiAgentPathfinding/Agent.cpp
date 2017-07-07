#include "Agent.h"
#include "AStar.h"
#include "Tile.h"
#include "GridMap.h"

Agent::Agent(GridMap* inMap, AStar* inSearch, int x, int y) : EObject(x, y)
{
	search = inSearch;
	map = inMap;
}

void Agent::Step()
{
	if (path.empty())
	{
		Tile* currentTile = map->getTileAt(x, y);
		Tile* randomTile = map->randomWalkableTile();
		path = search->findPath(currentTile, randomTile);

		if (path.empty()) return;
	}
	//else // move along current path
	{
		Tile* nextTile = path.back();
		path.pop_back();
		x = nextTile->x;
		y = nextTile->y;
	}
}