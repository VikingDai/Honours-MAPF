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

AStar::Path AStar::FindPath(Tile* start, Tile* goal, TileCosts& customCosts)
{
	// calculate time
	SEARCH_COUNT += 1;
	TIME_START = std::chrono::system_clock::now();

	Path path;

	if (!start || !goal || start == goal)
		return Path{ start };

	if (!start->isWalkable || !goal->isWalkable)
		return Path{ start };

	modifiedTiles.clear();
	OpenQueue open;

	open.push_back(new TileInfo(-1, start, 0, start->CalculateEstimate(0, 0, start, goal)));
	modifiedTiles.push_back(start);

	TileInfo* current = nullptr;

	while (!open.empty())
	{
		if (current)
			std::cout << "Currently: " << *current->tile << std::endl;

		current = open[open.size() - 1];

		std::cout << "\n### OPEN LIST ###" << std::endl;

		for (TileInfo* info : open)
		{
			std::cout << "\t" << *info->tile << " at Time " << info->timestep << " with Estimate " << info->estimate << std::endl;
		}

		std::cout << std::endl << std::endl;

		if (current->timestep > 500)
		{
			std::cout << "A* search timed out: took too long!" << std::endl;
			break;
		}

		std::cout << "A* CHOSE TO EXPAND: " << *current->tile << " at Time " << current->timestep << " with Estimate " << current->estimate << std::endl;

		open.pop_back();

		//current->color = vec3(0, 0, 1);

		if (current->tile == goal) // found path to goal!
			break;

		AddToOpen(open, current, gridMap->getTileRelativeTo(current->tile, 0, 1), start, goal, customCosts);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current->tile, 1, 0), start, goal, customCosts);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current->tile, 0, -1), start, goal, customCosts);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current->tile, -1, 0), start, goal, customCosts);
		AddToOpen(open, current, gridMap->getTileRelativeTo(current->tile, 0, 0), start, goal, customCosts);
	}

	int time = current->timestep;
	std::cout << "> FOUND GOAL: " << *current->tile << " at Time " << current->timestep << " with Estimate " << current->estimate << std::endl;

	// bulding path 2
	while (cameFrom.find(current) != cameFrom.end())
	{
		std::cout << "A* building path: " << *current->tile << " timestep: " << current->timestep << std::endl;
		path.push_front(current->tile);

		//if (cameFrom.count[current] == 0)
		//{
		//	std::cout << "ERROR" << std::endl;
		//	return Path{ start };
		//}

		current = cameFrom[current];
	}


	//Tile* tempTile = current->tile;

	//// rebuild the path
	//while (tempTile != start)
	//{
	//	std::cout << "A* building path: " << *tempTile << " timestep: " << time << std::endl;
	//	path.push_front(tempTile);

	//	if (!tempTile->parentsByTime.count(time))
	//	{
	//		std::cerr << "ERROR: Parent to goal is not valid" << std::endl;
	//		return Path{ start };
	//	}
	//	
	//	tempTile = tempTile->parentsByTime[time];
	//	time -= 1;
	//}

	// reset visited tiles
	for (Tile* tile : modifiedTiles) tile->Reset();

	// calculate time taken
	TIME_END = std::chrono::system_clock::now();
	std::chrono::duration<double> timeElapsed = TIME_END - TIME_START;
	TIME_ACC += timeElapsed.count(); 
	Stats::avgSearchTime = TIME_ACC / (double) SEARCH_COUNT;

	return path;
}

void AStar::AddToOpen(OpenQueue& open, TileInfo* currentInfo, Tile* tile, Tile* start, Tile* goal, TileCosts& customCosts)
{
	int timestep = currentInfo->timestep + 1;
	bool isWaitAction = currentInfo->tile == tile;

	float actionCost = isWaitAction ? 0.75f : 1.f;
	float newCost = currentInfo->cost + actionCost;

	Tile* from = currentInfo->tile;

	if ((tile && !tile->visited[timestep] && tile->isWalkable))
	{
		//std::cout << *tile << " at " << " time: " << timestep << std::endl;

		if (from == tile)
		{
			std::cout << "Stay Action!" << std::endl;
		}

		tile->visited[timestep] = true;
		modifiedTiles.push_back(tile);

		tile->parentsByTime[timestep] = from;
		
		float tileDist = 1;
		//float cost = from->cost[timestep] + tileDist; //+ tile->numberOfTimesVisited;
		//cost += tileDist;

		tile->numberOfTimesVisited += 1;
		//tile->color = vec3(tile->numberOfTimesVisited * 0.05f);

		//std::cout << *from << " " << *tile << " " << timestep << std::endl;

		if (!customCosts.empty())
		{
			//std::cout << "Checking timestep " << timestep << " " << *tile << std::endl;

			bool hasCustomCost = customCosts.count(timestep) && customCosts[timestep].count(tile);
			if (hasCustomCost) 
				newCost += customCosts[timestep][tile];

			if (hasCustomCost)
				std::cout << "Using cost!" << std::endl;
		}

		TileInfo* newTileInfo = new TileInfo(timestep, tile, newCost, tile->CalculateEstimate(timestep, newCost, start, goal));

		cameFrom[newTileInfo] = currentInfo;
		open.push_back(newTileInfo);
		//tile->color = vec3(0, 1, 1);
	}
	
	std::sort(open.begin(), open.end(), BaseHeuristic(timestep));
}

bool BaseHeuristic::operator()(TileInfo* A, TileInfo* B)
{
	return A->estimate > B->estimate;
}
