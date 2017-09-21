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
	timer.Begin();
	Path path;

	if (!start || !goal || start == goal || !start->isWalkable || !goal->isWalkable)
	{
		// no path was found, return a path of length 1 where the agent does not move
		path.push_front(start);
		return path;
	}

	LOCAL_EXPANDED = 0;

	for (Tile* tile : modifiedTiles)
		tile->ResetColor();
	modifiedTiles.clear();
	OpenQueue open;

	start->CalculateEstimate(0, goal);
	open.push_back(start);
	modifiedTiles.push_back(start);

	Tile* current = start;
	while (!open.empty())
	{
		std::sort(open.begin(), open.end(), Heuristic());
		current = open.back();
		open.pop_back();

		current->SetColor(sf::Color::Red);

		if (current == goal) // found path to the goal
			break;

		current->hasBeenExpanded = true;

		// add the neighbors of the current tile (up, down, left, right) to the open list
		AddNeighbor(open, current, gridMap->GetTileRelativeTo(current, 0, 1), start, goal); // up
		AddNeighbor(open, current, gridMap->GetTileRelativeTo(current, 1, 0), start, goal); // right
		AddNeighbor(open, current, gridMap->GetTileRelativeTo(current, 0, -1), start, goal); // down
		AddNeighbor(open, current, gridMap->GetTileRelativeTo(current, -1, 0), start, goal); // left
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

	timer.End();

	std::cout << "AStar expanded <" << LOCAL_EXPANDED << "> | Took " << timer.GetTimeElapsed() << " | " << timer.GetTimeElapsed() / LOCAL_EXPANDED << " per expansion  | found path size " << path.size() << std::endl;

	

	return path;
}

void AStar::AddNeighbor(OpenQueue& open, Tile* current, Tile* neighbor, Tile* start, Tile* goal)
{
	assert(current && start && goal);

	if (!neighbor || // this will occur when a tile is out of bounds
		neighbor->hasBeenExpanded || // skip neighbors which have already been expanded
		!neighbor->isWalkable) // don't expand obstacles
		return;

	LOCAL_EXPANDED += 1;

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

		neighbor->SetColor(sf::Color::Green);

		modifiedTiles.push_back(neighbor);
	}
}