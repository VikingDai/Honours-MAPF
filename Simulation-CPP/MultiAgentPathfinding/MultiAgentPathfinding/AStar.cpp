#include "AStar.h"
#include <algorithm>
#include <stack>

#define DEBUG_ASTAR 0

AStar::AStar(GridMap* inGridMap)
{
	gridMap = inGridMap;
}

AStar::Path AStar::FindPath(Tile* start, Tile* goal)
{
	Path path;

	if (!start || !goal || start == goal || !start->isWalkable || !goal->isWalkable)
	{
		// no path was found, return a path of length 1 where the agent does not move
		path.push_front(start);
		return path;
	}

	std::vector<Tile*> modifiedTiles;
	OpenQueue open;

	start->CalculateEstimate(0, goal);
	open.push_back(start);
	modifiedTiles.push_back(start);

	Tile* current = start;
	while (!open.empty())
	{
		std::sort(open.begin(), open.end(), Heuristic());
		current = open.front();
		open.erase(open.begin());

		if (current == goal) // found path to the goal
			break;

		current->hasBeenExpanded = true;

		// add the neighbors of the current tile (up, down, left, right) to the open list
		AddNeighbor(open, modifiedTiles, current, gridMap->GetTileRelativeTo(current, 0, 1), start, goal); // up
		AddNeighbor(open, modifiedTiles, current, gridMap->GetTileRelativeTo(current, 1, 0), start, goal); // right
		AddNeighbor(open, modifiedTiles, current, gridMap->GetTileRelativeTo(current, 0, -1), start, goal); // down
		AddNeighbor(open, modifiedTiles, current, gridMap->GetTileRelativeTo(current, -1, 0), start, goal); // left
	}

	// build the path
	while (current->parent != nullptr)
	{
		path.push_front(current);
		current = current->parent;
	}

	// reset all modified tiles
	for (Tile* tile : modifiedTiles) 
		tile->Reset();

	return path;
}

void AStar::AddNeighbor(OpenQueue& open, std::vector<Tile*>& modifiedTiles, Tile* current, Tile* neighbor, Tile* start, Tile* goal)
{
	assert(current && start && goal);

	if (!neighbor || // this will occur when a tile is out of bounds
		neighbor->hasBeenExpanded || // skip neighbors which have already been expanded
		!neighbor->isWalkable) // don't expand obstacles
		return;

	float newNeighborCost = current->cost + 1;

	if (neighbor->isInOpen)
	{
#if DEBUG_ASTAR
		std::cout << "UPDATED " << *neighbor << " in open list." << std::endl;
#endif

		// update neighbor in the fringe by checking if this new parent tile is better
		bool newCostIsBetter = newNeighborCost < neighbor->cost;
		if (newCostIsBetter) // only accept this new path if the cost is less
		{
			neighbor->CalculateEstimate(newNeighborCost, goal);
			neighbor->parent = current;
		}
	}
	else // add neighbor to the fringe
	{
#if DEBUG_ASTAR
		std::cout << "ADDED " << *neighbor << " to open list." << std::endl;
#endif

		neighbor->isInOpen = true;
		neighbor->CalculateEstimate(newNeighborCost, goal);
		neighbor->parent = current;

		open.push_back(neighbor);

		neighbor->color = sf::Color(0, 1, 0);

		modifiedTiles.push_back(neighbor);
	}
}