#pragma once

#include "EObject.h"
#include <iostream>


class Tile : public EObject
{
public:
	bool isWalkable;
	bool visited;
	float estimate;
	float cost;
	float heuristic;
	Tile* parent;

	vec3 color;

	Tile(int x, int y, bool inIsWalkable);
	void Reset();
	void CalculateEstimate(float cost, Tile* start, Tile* goal);

	friend std::ostream& operator<<(std::ostream& os, Tile& tile);
};