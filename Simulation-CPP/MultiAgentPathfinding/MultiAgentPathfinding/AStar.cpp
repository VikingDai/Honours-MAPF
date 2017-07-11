#include "AStar.h"
#include "Tile.h"
#include "GridMap.h"
#include <deque>

AStar::AStar(GridMap* inGridMap)
{
	gridMap = inGridMap;
}

AStar::~AStar()
{
}

std::deque<Tile*> AStar::findPath(Tile* start, Tile* goal)
{
	std::deque<Tile*> path;
	//std::cout << "Finding path from " << *start << " to " << *goal;

	if (!start || !goal || start == goal)
		return path;

	if (!start->isWalkable || !goal->isWalkable)
		return path;

	visited.clear();
	OpenQueue open;

	open.push_back(start);
	visited.push_back(start);

	Tile* current = nullptr;

	while (!open.empty())
	{
		current = open[open.size() - 1];
		open.pop_back();
		//open.pop();

		//current->color = vec3(0, 0, 1);

		if (current == goal) // found path to goal!
			break;

		AddToOpen(open, current, gridMap->getTileRelativeTo(current, 0, 1), start, goal);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current, 1, 0), start, goal);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current, 0, -1), start, goal);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current, -1, 0), start, goal);
	}

	// rebuild the path
	while (true)
	{
		path.push_front(current);

		if (!current->parent) std::cerr << "ERROR: Parent to goal is not valid" << std::endl;
		
		if (current == start)
			break;

		current = current->parent;
	}

	// reset visited tiles
	for (Tile* tile : visited) tile->Reset();

	// print the path
	/*for (Tile* tile : path)
		std::cout << *tile << std::endl;*/

	return path;
}

void AStar::AddToOpen(OpenQueue& open, Tile* from, Tile* tile, Tile* start, Tile* goal)
{
	if (tile && !tile->visited && tile->isWalkable)
	{
		tile->visited = true;
		visited.push_back(tile);

		open.push_back(tile);
		tile->parent = from;
		tile->CalculateEstimate(from->cost + 1, start, goal);

		//tile->color = vec3(0, 1, 1);
	}
	
	std::sort(open.begin(), open.end(), BaseHeuristic());
}

bool BaseHeuristic::operator()(Tile* A, Tile* B)
{
	return A->estimate > B->estimate;
}
