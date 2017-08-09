#include "AStar.h"

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
		path.push(start);
		return path;
	}

	std::vector<Tile*> modifiedTiles;
	OpenQueue open;

	start->CalculateEstimate(0, goal);
	open.push(start);
	modifiedTiles.push_back(start);

	Tile* current = start;
	while (!open.empty())
	{
		current = open.top();
		std::cout << "Expanding " << *current << std::endl;
		open.pop();
		current->hasBeenExpanded = true;
		current->color = glm::vec3(1, 0, 0);

		if (current == goal) // found path to the goal
			break;

		// add the neighbors of the current tile (up, down, left, right) to the open list
		AddNeighbor(open, current, gridMap->getTileRelativeTo(current, 0, 1), start, goal); // up
		AddNeighbor(open, current, gridMap->getTileRelativeTo(current, 1, 0), start, goal); // right
		AddNeighbor(open, current, gridMap->getTileRelativeTo(current, 0, -1), start, goal); // down
		AddNeighbor(open, current, gridMap->getTileRelativeTo(current, -1, 0), start, goal); // left
	}

	// build the path
	while (current != start)
	{
		path.push(current);
		current = current->parent;
	}

	// reset all modified tiles
	for (Tile* tile : modifiedTiles) tile->Reset();

	return path;
}

void AStar::AddNeighbor(OpenQueue& open, Tile* current, Tile* neighbor, Tile* start, Tile* goal)
{
	assert(current && start && goal);

	if (!neighbor) // this will occur when a tile is out of bounds
		return; 
	if (neighbor->hasBeenExpanded) return; // skip neighbors which have already been expanded

	float newNeighborCost = current->cost + 1;

	if (neighbor->isInOpen) 
	{
		std::cout << "UPDATED " << *neighbor << " in open list." << std::endl;

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
		std::cout << "ADDED " << *neighbor << " to open list." << std::endl;

		neighbor->isInOpen = true;
		neighbor->CalculateEstimate(newNeighborCost, goal);
		neighbor->parent = current;

		open.push(neighbor);

		neighbor->color = glm::vec3(0, 1, 0);
	}
}
