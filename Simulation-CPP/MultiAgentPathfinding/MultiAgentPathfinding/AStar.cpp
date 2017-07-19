#include "AStar.h"
#include "Tile.h"
#include "GridMap.h"
#include <deque>
#include <chrono>
#include <ctime>
#include "Statistics.h"

double TIME_ACC = 0;
int SEARCH_COUNT = 0;
std::chrono::time_point<std::chrono::system_clock> TIME_START, TIME_END;

AStar::AStar(GridMap* inGridMap)
{
	gridMap = inGridMap;
}

AStar::~AStar()
{
}

AStar::Path AStar::findPath(Tile* start, Tile* goal)
{
	// calculate time
	SEARCH_COUNT += 1;
	TIME_START = std::chrono::system_clock::now();

	Path path;

	if (!start || !goal || start == goal)
		return Path{ start };

	if (!start->isWalkable || !goal->isWalkable)
		return Path{ start };

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
	while (current != start)
	{
		path.push_front(current);

		if (!current->parent)
		{
			std::cerr << "ERROR: Parent to goal is not valid" << std::endl;
			return Path{ start };
		}
		
		current = current->parent;
	}

	// reset visited tiles
	for (Tile* tile : visited) tile->Reset();

	// print the path
	/*for (Tile* tile : path)
		std::cout << *tile << std::endl;*/

	// calculate time taken
	TIME_END = std::chrono::system_clock::now();
	std::chrono::duration<double> timeElapsed = TIME_END - TIME_START;
	TIME_ACC += timeElapsed.count(); 
	Stats::avgSearchTime = TIME_ACC / (double) SEARCH_COUNT;

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
