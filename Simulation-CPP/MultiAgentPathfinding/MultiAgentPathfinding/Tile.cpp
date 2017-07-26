#include "Tile.h"
#include <iostream>
#include <glm/vec3.hpp>

Tile::Tile(int x, int y, bool inIsWalkable) : EObject(x, y)
{
	isWalkable = inIsWalkable;
	Reset();
}

void Tile::Reset()
{
	visited = false;
	estimate = 0;
	cost = 0;
	heuristic = 0;
	parentsByTime.clear();
	timeVisited = -1;
	numberOfTimesVisited = 0;
}

void Tile::CalculateEstimate(float inCost, Tile* start, Tile* goal)
{
	cost = inCost;
	float dx = goal->x - x;
	float dy = goal->y - y;
	heuristic = sqrt(dx * dx + dy * dy);
	estimate = cost + heuristic;
}

std::ostream& operator<<(std::ostream& os, Tile& tile)
{
	os << "Tile (" << tile.x << "," << tile.y << ")";
	return os;
}
