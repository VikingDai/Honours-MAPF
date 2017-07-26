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

AStar::Path AStar::findPath(Tile* start, Tile* goal, TileCosts& customCosts)
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

		std::cout << "AStar expanding " << *current << std::endl;

		open.pop_back();

		//current->color = vec3(0, 0, 1);

		if (current == goal) // found path to goal!
			break;

		AddToOpen(open, current, gridMap->getTileRelativeTo(current, 0, 1), start, goal, customCosts);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current, 1, 0), start, goal, customCosts);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current, 0, -1), start, goal, customCosts);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current, -1, 0), start, goal, customCosts);
		//AddToOpen(open, current, gridMap->getTileRelativeTo(current, 0, 0), start, goal, customCosts);
	}

	int time = current->timeVisited;

	// rebuild the path
	while (current != start)
	{
		path.push_front(current);

		if (!current->parentsByTime.count(time))
		{
			std::cerr << "ERROR: Parent to goal is not valid" << std::endl;
			return Path{ start };
		}
		
		current = current->parentsByTime[time];
		time -= 1;

		std::cout << "A* building path: " << *current << " timestep: " << time << std::endl;
	}

	// reset visited tiles
	for (Tile* tile : visited) tile->Reset();

	// calculate time taken
	TIME_END = std::chrono::system_clock::now();
	std::chrono::duration<double> timeElapsed = TIME_END - TIME_START;
	TIME_ACC += timeElapsed.count(); 
	Stats::avgSearchTime = TIME_ACC / (double) SEARCH_COUNT;

	return path;
}

void AStar::AddToOpen(OpenQueue& open, Tile* from, Tile* tile, Tile* start, Tile* goal, TileCosts& customCosts)
{
	int newTime = from->timeVisited + 1;

	if ((tile && !tile->visited && tile->isWalkable) || from == tile)
	{
		tile->visited = true;
		visited.push_back(tile);

		open.push_back(tile);
		tile->parentsByTime[newTime] = from;
		tile->timeVisited = newTime;
		
		float tileDist = 1;
		float cost = from->cost + tileDist + tile->numberOfTimesVisited;

		tile->numberOfTimesVisited += 1;
		tile->color = vec3(tile->numberOfTimesVisited * 0.05f);

		std::cout << *from << " " << *tile << " " << newTime << std::endl;

		if (!customCosts.empty())
		{
			std::cout << "Checking timestep " << newTime << " " << *tile << std::endl;

			bool hasCustomCost = customCosts.count(newTime) && customCosts[newTime].count(tile);
			if (hasCustomCost) 
				cost += customCosts[newTime][tile];

			if (hasCustomCost)
				std::cout << "Using cost!" << std::endl;
		}

		tile->CalculateEstimate(cost, start, goal);

		//tile->color = vec3(0, 1, 1);
	}
	
	std::sort(open.begin(), open.end(), BaseHeuristic());
}

bool BaseHeuristic::operator()(Tile* A, Tile* B)
{
	return A->estimate > B->estimate;
}
