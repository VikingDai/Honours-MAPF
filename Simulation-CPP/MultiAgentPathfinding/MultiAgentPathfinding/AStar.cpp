#include "AStar.h"
#include "Tile.h"
#include "GridMap.h"

AStar::AStar(GridMap* inGridMap)
{
	gridMap = inGridMap;
}

AStar::~AStar()
{
}

std::vector<Tile*> AStar::findPath(Tile* start, Tile* goal)
{
	std::vector<Tile*> path;

	if (!start || !goal)
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

		current->color = vec3(0, 0, 1);

		if (current == goal)
		{
			std::cout << "Found path to goal!" << std::endl;
			break;
		}

		AddToOpen(open, current, gridMap->getTileRelativeTo(current, 0, 1), start, goal);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current, 1, 0), start, goal);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current, 0, -1), start, goal);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current, -1, 0), start, goal);
	}

	// rebuild the path
	while (true)
	{
		std::cout << "Path: " << current->x << "," << current->y << std::endl;
		path.push_back(current);
		if (!current->parent)
		{
			std::cerr << "ERROR: Parent to goal is not valid" << std::endl;
		}
			
		if (current->parent == start)
		{
			std::cout << "blururre" << std::endl;
			break;
		}

		current = current->parent;
	}

	// reset visited tiles
	for (Tile* tile : visited) tile->Reset();

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

		tile->color = vec3(0, 1, 1);
	}
	
	std::sort(open.begin(), open.end(), BaseHeuristic());
}

bool BaseHeuristic::operator()(Tile* A, Tile* B)
{
	return A->estimate > B->estimate;
}
