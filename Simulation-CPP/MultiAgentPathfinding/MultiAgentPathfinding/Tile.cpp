#include "Tile.h"
#include <iostream>
#include <glm/vec3.hpp>

std::vector<Tile*> Tile::dirtyTiles;

Tile::Tile(int x, int y, int index, bool inIsWalkable) 
	: EObject(x, y, index), 
	isDirty(false), isWalkable(inIsWalkable)
{
	ResetColor();
	Reset();
}

void Tile::Reset()
{
	isInOpen = false;
	hasBeenExpanded = false;
	estimate = 0;
	cost = 0;
	heuristic = 0;
	parent = nullptr;

	visitedAtTime.clear();

	numberOfTimesVisited = 0;
}

std::ostream& operator<<(std::ostream& os, Tile& tile)
{
 	os << "Tile " << tile.index << " (" << tile.x << "," << tile.y << ")";
	return os;
}

float Tile::CalculateEstimate(float inCost, Tile* goal)
{
	cost = inCost;

	// using manhattan heuristic
	float dx = goal->x - x;
	float dy = goal->y - y;
	heuristic = abs(dx) + abs(dy);

	// update estimate : f = g + h
	estimate = cost + heuristic;
	return estimate;
}

void Tile::SetObstacle()
{
	isWalkable = false;
	color = sf::Color::Black;
}

void Tile::SetColor(sf::Color inColor)
{
	color = inColor;
	if (!isDirty)
	{
		isDirty = true;
		dirtyTiles.push_back(this);
	}
}