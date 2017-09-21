#pragma once
#include "Tile.h"
class Heuristics
{
public:
	static float Manhattan(Tile* current, Tile* goal)
	{
		float dx = goal->x - current->x;
		float dy = goal->y - current->y;
		return abs(dx) + abs(dy);
	};
};

