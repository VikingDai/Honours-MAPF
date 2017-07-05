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

	std::priority_queue<Tile*> open;
	closed.clear();

	open.push(start);
	closed.push_back(start);

	Tile* current = nullptr;

	while (!open.empty())
	{
		current = open.top();
		open.pop();

		current->color = vec3(0, 0, 1);

		std::cout << "Current: " << current->x << "," << current->y << std::endl;

		if (current == goal)
		{
			break;
		}

		AddToOpen(open, current, 0, 1);
		AddToOpen(open, current, 1, 0);
		AddToOpen(open, current, 0, -1);
		AddToOpen(open, current, -1, 0);
	}

	// rebuild the path
	while (true)
	{
		std::cout << "\tPath: " << current->x << "," << current->y << std::endl;
		path.push_back(current);
		if (!current->parent || current->parent == start) break;

		current = current->parent;

		current->color = vec3(0, 1, 0);
	}

	std::cout << "Found Path of length: " << path.size() << std::endl;

	for (Tile* tile : closed)
	{
		tile->Reset();
	}

	return path;
}

void AStar::AddToOpen(std::priority_queue<Tile*>& open, Tile* tile, int x, int y)
{
	Tile* relative = gridMap->getTileRelativeTo(tile, x, y);
	if (relative && !relative->visited)
	{
		open.push(relative);
		relative->visited = true;
		relative->parent = tile;
		closed.push_back(relative);
		
	}
}
