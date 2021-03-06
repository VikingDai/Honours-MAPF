#pragma once

#include "EObject.h"
#include <iostream>
#include <map>
#include <vector>

#include <SFML/Graphics/Color.hpp>


class Tile : public EObject
{
public:
	static std::vector<Tile*> dirtyTiles;

	// from at time
	std::map<Tile*, std::map<int, bool>> visitedAtTime;

	bool isWalkable;


	bool hasBeenExpanded;
	bool isInOpen;

	bool isDirty;

	float f;
	float g;
	float h;
	Tile* parent;

	std::vector<Tile*> sameCostParents;

	int numberOfTimesVisited;

private:
	sf::Color color;

public:
	void SetColor(sf::Color color);
	const sf::Color GetColor() { return color; }
	const sf::Color ResetColor() { SetColor(isWalkable ? sf::Color::White : sf::Color(50, 50, 50)); return GetColor(); }

	Tile(int x, int y, int index, bool inIsWalkable);
	void Reset();

	float CalculateEstimate(float inCost, Tile* goal);

	friend std::ostream& operator<<(std::ostream& os, Tile& tile);

	void SetObstacle();


public:
	int priority = INT_MAX;
	int GetPriority() { return priority; }
	void SetPriority(int priority)
	{
		this->priority = priority;
	}

	bool operator<(const Tile& other)
	{
		if (f == other.f)
			return g > other.g;

		return f < other.f;
	}

	bool operator>(const Tile& other)
	{
		if (f == other.f)
			return g < other.g;

		return f > other.f;
	}
};