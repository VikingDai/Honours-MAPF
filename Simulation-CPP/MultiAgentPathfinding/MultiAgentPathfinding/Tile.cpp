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
