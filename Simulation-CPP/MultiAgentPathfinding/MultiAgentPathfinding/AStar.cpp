#include "AStar.h"
#include "Tile.h"
#include "GridMap.h"
#include <deque>
#include <chrono>
#include <ctime>
#include "Statistics.h"

int NODES_EXPANDED = 0;

#define DEBUG_VERBOSE 0

AStar::AStar(GridMap* inGridMap)
{
	gridMap = inGridMap;
}

AStar::~AStar()
{
}

AStar::Path AStar::FindPath(Tile* start, Tile* goal, TileCosts& customCosts)
{
	timer.Begin();
	NODES_EXPANDED = 0;
	
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
	Tile* currentTile = nullptr;

	while (!open.empty())
	{
#if DEBUG_VERBOSE
		if (current)
			std::cout << "Currently: " << current << std::endl;
		std::cout << "\n### OPEN LIST ###" << std::endl;
		for (TileInfo* info : open)
		{
			std::cout << "\t" << *info->tile << " at Time " << info->timestep << " with Estimate " << info->estimate << std::endl;
		}
		std::cout << std::endl << std::endl;
#endif
		current = open[open.size() - 1];
		NODES_EXPANDED += 1;

		if (current->timestep > 500)
		{
			std::cout << "A* search timed out: took too long!" << std::endl;
			break;
		}

#if DEBUG_VERBOSE
		std::cout << "A* CHOSE TO EXPAND: " << *current->tile << " at Time " << current->timestep << " with Estimate " << current->estimate << std::endl;
#endif

		open.pop_back();

		//current->color = vec3(0, 0, 1);

		if (current->tile == goal) // found path to goal!
			break;

		AddToOpen(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, 0, 1), start, goal, customCosts);
		AddToOpen(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, 1, 0), start, goal, customCosts);
		AddToOpen(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, 0, -1), start, goal, customCosts);
		AddToOpen(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, -1, 0), start, goal, customCosts);
		AddToOpen(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, 0, 0), start, goal, customCosts);
	}

	// build the path
	while (cameFrom.find(current) != cameFrom.end())
	{
#if DEBUG_VERBOSE
		std::cout << "A* building path: " << *current->tile << " timestep: " << current->timestep << std::endl;
#endif
		path.push_front(current->tile);

		current = cameFrom[current];
	}

	assert(current->tile == start);
	
	// calculate time taken
	timer.End();
	Stats::avgSearchTime = timer.getAvgTime();

	std::cout << "Search visited: " << modifiedTiles.size() << " tiles | Expanded: " << NODES_EXPANDED << " tiles. Took " << timer.getTimeElapsed() << " ms" << std::endl;

	// reset visited tiles
	for (Tile* tile : modifiedTiles) tile->Reset();

	return path;
}

void AStar::AddToOpen(OpenQueue& open, TileInfo* currentInfo, Tile* fromTile, Tile* tile, Tile* start, Tile* goal, TileCosts& customCosts)
{
	int timestep = currentInfo->timestep + 1;
	bool isWaitAction = currentInfo->tile == tile;

	float actionCost = isWaitAction ? 0.75f : 1.f;
	float newCost = currentInfo->cost + actionCost;

	Tile* from = currentInfo->tile;

	if ((tile && !tile->visited[timestep] && tile->isWalkable))
	{
#if DEBUG_VERBOSE
		std::cout << *tile << " at " << " time: " << timestep << std::endl;
#endif

		tile->visited[timestep] = true;
		modifiedTiles.push_back(tile);

		tile->parentsByTime[timestep] = from;
		
		float tileDist = 1;
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

		//float dx = goal->x - tile->x;
		//float dy = goal->y - tile->y;
		//float heuristic = sqrt(dx * dx + dy * dy);

		//tile->spatialEstimates[timestep].CalculateEstimate(newCost, heuristic);
		//tile->parentsByTime[timestep] = fromTile;

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
