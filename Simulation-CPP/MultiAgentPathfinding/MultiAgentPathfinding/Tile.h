#pragma once

#include "EObject.h"
#include <iostream>
#include <map>

#define MAX_PATH_SIZE 10000


class Tile : public EObject
{
public:
	bool isWalkable;
	bool visited[MAX_PATH_SIZE];
	float estimate[MAX_PATH_SIZE];
	float cost[MAX_PATH_SIZE];
	float heuristic[MAX_PATH_SIZE];

	int numberOfTimesVisited;
	std::map<int, Tile*> parentsByTime;

	vec3 color;

	Tile(int x, int y, bool inIsWalkable);
	void Reset();
	float CalculateEstimate(int timestep, float cost, Tile* start, Tile* goal);

	friend std::ostream& operator<<(std::ostream& os, Tile& tile);
};