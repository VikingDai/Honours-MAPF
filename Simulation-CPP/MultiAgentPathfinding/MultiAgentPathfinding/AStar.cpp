#include "AStar.h"
#include <algorithm>
#include <stack>

AStar::AStar(GridMap* inGridMap)
{
	gridMap = inGridMap;
}

//AStar::Path AStar::FindPath(Tile* start, Tile* goal)
//{
//	Path path;
//
//	if (!start || !goal || start == goal || !start->isWalkable || !goal->isWalkable)
//	{
//		// no path was found, return a path of length 1 where the agent does not move
//		path.push_front(start);
//		return path;
//	}
//
//	std::vector<Tile*> modifiedTiles;
//	OpenQueue open;
//
//	start->CalculateEstimate(0, goal);
//	open.push(start);
//	modifiedTiles.push_back(start);
//
//	Tile* current = start;
//	while (!open.empty())
//	{
//		current = open.top();
//		open.pop();
//		current->hasBeenExpanded = true;
//		//current->color = glm::vec3(1, 0, 0);
//
//		if (current == goal) // found path to the goal
//		{
//			std::cout << "FOUND goal!" << std::endl;
//			break;
//		}
//
//		// add the neighbors of the current tile (up, down, left, right) to the open list
//		AddNeighbor(open, current, gridMap->getTileRelativeTo(current, 0, 1), start, goal); // up
//		AddNeighbor(open, current, gridMap->getTileRelativeTo(current, 1, 0), start, goal); // right
//		AddNeighbor(open, current, gridMap->getTileRelativeTo(current, 0, -1), start, goal); // down
//		AddNeighbor(open, current, gridMap->getTileRelativeTo(current, -1, 0), start, goal); // left
//	}
//
//	// build the path
//	while (current != start)
//	{
//		path.push_front(current);
//		current = current->parent;
//	}
//
//	// reset all modified tiles
//	for (Tile* tile : modifiedTiles) tile->Reset();
//
//	return path;
//}

//void AStar::AddNeighbor(OpenQueue& open, Tile* current, Tile* neighbor, Tile* start, Tile* goal)
//{
//	assert(current && start && goal);
//
//	if (!neighbor || // this will occur when a tile is out of bounds
//		neighbor->hasBeenExpanded || // skip neighbors which have already been expanded
//		!neighbor->isWalkable) // don't expand obstacles
//		return;
//
//	float newNeighborCost = current->cost + 1;
//
//	if (neighbor->isInOpen)
//	{
//		std::cout << "UPDATED " << *neighbor << " in open list." << std::endl;
//
//		// update neighbor in the fringe by checking if this new parent tile is better
//		bool newCostIsBetter = newNeighborCost < neighbor->cost;
//		if (newCostIsBetter) // only accept this new path if the cost is less
//		{
//			neighbor->CalculateEstimate(newNeighborCost, goal);
//			neighbor->parent = current;
//		}
//	}
//	else // add neighbor to the fringe
//	{
//		std::cout << "ADDED " << *neighbor << " to open list." << std::endl;
//
//		neighbor->isInOpen = true;
//		neighbor->CalculateEstimate(newNeighborCost, goal);
//		neighbor->parent = current;
//
//		open.push(neighbor);
//
//		neighbor->color = glm::vec3(0, 1, 0);
//	}
//}

std::vector<AStar::Path> AStar::FindPaths(Tile* start, Tile* goal)
{
	std::vector<AStar::Path> paths;

	if (!start || !goal || start == goal || !start->isWalkable || !goal->isWalkable)
	{
		// no path was found, return a path of length 1 where the agent does not move
		Path stayPath;
		stayPath.push_front(start);
		paths.push_back(stayPath);
		return paths;
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

		//std::cout << "Expanding " << *current << "(" << open.size() << ")" << std::endl;
		
		current->hasBeenExpanded = true;
		current->color = glm::vec3(1, 0, 0);

		//if (current == goal) // found path to the goal
		//	break;

		// add the neighbors of the current tile (up, down, left, right) to the open list
		AddSameCostNeighbor(open, current, gridMap->getTileRelativeTo(current, 0, 1), start, goal); // up
		AddSameCostNeighbor(open, current, gridMap->getTileRelativeTo(current, 1, 0), start, goal); // right
		AddSameCostNeighbor(open, current, gridMap->getTileRelativeTo(current, 0, -1), start, goal); // down
		AddSameCostNeighbor(open, current, gridMap->getTileRelativeTo(current, -1, 0), start, goal); // left
	}

	std::cout << "# REBUILDING PATH " << std::endl;

	std::stack<Path> pathStack;
	Path endPath;
	endPath.push_front(goal);
	pathStack.push(endPath);
	
	// build the path
	while (!pathStack.empty())
	{
		Path currentPath = pathStack.top();
		pathStack.pop();

		Tile* next = currentPath.front();
		//std::cout << "Rebuilding path " << *next << std::endl;

		// we found one path, but we still need to check the parents we have not explored!
		if (next == start)
		{
			paths.push_back(currentPath);
			continue;
		}

		for (Tile* parent : next->sameCostParents)
		{
			Path newPath = currentPath;
			newPath.push_front(parent);

			//std::cout << "Expanding path to " << *parent << std::endl;
			pathStack.push(newPath);
		}
	}

	std::cout << "Successfully found " << paths.size() << " paths!" << std::endl;

	// reset all modified tiles
	for (Tile* tile : modifiedTiles) tile->Reset();

	return paths;
}

void AStar::AddSameCostNeighbor(OpenQueue& open, Tile* current, Tile* neighbor, Tile* start, Tile* goal)
{
	if (!neighbor || // this will occur when a tile is out of bounds
		neighbor->hasBeenExpanded || // skip neighbors which have already been expanded
		!neighbor->isWalkable) // don't expand obstacles
		return;

	float newNeighborCost = current->cost + 1;

	if (neighbor->sameCostParents.empty()) // no parent assigned, add it straight to the fringe
	{
		std::cout << "ADDED " << *neighbor << " to open list." << std::endl;

		neighbor->isInOpen = true;
		neighbor->CalculateEstimate(newNeighborCost, goal);
		neighbor->sameCostParents.push_back(current);

		open.push_back(neighbor);

		neighbor->color = glm::vec3(0, 1, 0);
	}
	else
	{
		float parentCost = neighbor->sameCostParents[0]->cost;
		std::cout << "Parent cost: " << parentCost << " new cost: " << newNeighborCost << std::endl;

		// update neighbor in the fringe by checking if this new parent tile is better
		if (current->cost < parentCost) // only accept this new path if the cost is less
		{
			neighbor->CalculateEstimate(newNeighborCost, goal);

			neighbor->sameCostParents.clear();
			neighbor->sameCostParents.push_back(current);
		}
		else if (current->cost == parentCost) // parent has the same cost, add it to the same cost list
		{
			std::cout << "FOUND A SAME COST PARENT! " << *neighbor << " in open list." << std::endl;
			neighbor->sameCostParents.push_back(current);
		}
		else
		{
			std::cout << "Parent is BIGGER, ignore this parent" << std::endl;
		}
	}
}