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
	parent = nullptr;
}

void Tile::CalculateEstimate(float inCost, Tile* start, Tile* goal)
{
	cost = inCost;
	float dx = goal->x - x;
	float dy = goal->y - y;
	heuristic = sqrt(dx * dx + dy * dy);
	estimate = cost + heuristic;
}
