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
	memset(visited, false, MAX_PATH_SIZE);
	memset(estimate, 0, MAX_PATH_SIZE);
	memset(cost, 0, MAX_PATH_SIZE);
	memset(heuristic, 0, MAX_PATH_SIZE);
	//visited = false;
	//estimate = 0;
	//cost = 0;
	//heuristic = 0;
	parentsByTime.clear();
	numberOfTimesVisited = 0;
}

float Tile::CalculateEstimate(int timestep, float inCost, Tile* start, Tile* goal)
{
	//cost = inCost;
	//float dx = goal->x - x;
	//float dy = goal->y - y;
	//heuristic = sqrt(dx * dx + dy * dy);
	//estimate = cost + heuristic;

	cost[timestep] = inCost;
	float dx = goal->x - x;
	float dy = goal->y - y;
	heuristic[timestep] = sqrt(dx * dx + dy * dy);
	estimate[timestep] = inCost + heuristic[timestep];

	return estimate[timestep];
}

std::ostream& operator<<(std::ostream& os, Tile& tile)
{
 	os << "Tile (" << tile.x << "," << tile.y << ")";
	return os;
}

float Estimate::CalculateEstimate(float inCost, float inHeuristic)
{
	cost = inCost;
	heuristic = inHeuristic;
	estimate = cost + heuristic;
	return estimate;
}
