#pragma once

#include "EObject.h"
#include <iostream>
#include <map>


class Tile : public EObject
{
public:
	bool isWalkable;
	bool visited;
	float estimate;
	float cost;
	float heuristic;

	int numberOfTimesVisited;
	int timeVisited;
	std::map<int, Tile*> parentsByTime;

	vec3 color;

	Tile(int x, int y, bool inIsWalkable);
	void Reset();
	void CalculateEstimate(float cost, Tile* start, Tile* goal);

	friend std::ostream& operator<<(std::ostream& os, Tile& tile);
};