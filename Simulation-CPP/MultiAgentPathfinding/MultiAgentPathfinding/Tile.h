#pragma once

#include "EObject.h"
#include <iostream>
#include <map>
#include <vector>

struct Estimate
{
	int estimate;
	int cost;
	int heuristic;

	float GetEstimate() { return estimate; }
	float GetCost() { return estimate; }
	float GetHeuristic() { return heuristic; }

	float CalculateEstimate(float cost, float heuristic);

	Estimate() { Reset(); }
	void Reset() { estimate = cost = heuristic = 0; }
};

class Tile : public EObject

{
public:

	// from at time
	std::map<Tile*, std::map<int, bool>> visitedAtTime;

	bool isWalkable;


	bool hasBeenExpanded;
	bool isInOpen;

	float estimate;
	float cost;
	float heuristic;
	Tile* parent;

	std::vector<Tile*> sameCostParents;

	int numberOfTimesVisited;

	vec3 color;

	Tile(int x, int y, bool inIsWalkable);
	void Reset();

	float CalculateEstimate(float inCost, Tile* goal);

	friend std::ostream& operator<<(std::ostream& os, Tile& tile);
};