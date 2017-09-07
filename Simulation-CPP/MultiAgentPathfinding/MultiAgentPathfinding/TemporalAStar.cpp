#include "TemporalAStar.h"
#include "Tile.h"
#include "GridMap.h"
#include <deque>
#include <chrono>
#include <ctime>
#include "Statistics.h"

int NODES_EXPANDED = 0;

#define DEBUG_VERBOSE 1
#define TEST_GENERATE_DIFFERENT 1

TemporalAStar::TemporalAStar(GridMap* inGridMap)
{
	SetGridMap(inGridMap);
}

TemporalAStar::~TemporalAStar()
{
}

MAPF::Path TemporalAStar::FindPath(Tile* start, Tile* goal, TileCosts& customCosts)
{
	timer.Begin();
	NODES_EXPANDED = 0;

	MAPF::Path path;

	if (!start || !goal || start == goal)
		return MAPF::Path{ start };

	if (!start->isWalkable || !goal->isWalkable)
		return MAPF::Path{ start };

	modifiedTiles.clear();
	OpenQueue open;

	TileTime* startInfo = new TileTime();
	startInfo->SetInfo(-1, start, 0, start->CalculateEstimate(0, goal));

	//open.push_back(startInfo);
	open.push(startInfo);

	modifiedTiles.push_back(start);

	TileTime* current = nullptr;
	Tile* currentTile = nullptr;

	while (!open.empty())
	{
#if DEBUG_VERBOSE
		if (current)
			std::cout << "Currently: " << current << std::endl;
		//std::cout << "\n### OPEN LIST ###" << std::endl;
		//for (TileTime* info : open)
		//{
		//	std::cout << "\t" << *info->tile << " at Time " << info->timestep << " with Estimate " << info->estimate << std::endl;
		//}
		//std::cout << std::endl << std::endl;
#endif

		current = open.top();

		NODES_EXPANDED += 1;
		if (current->tile == goal) // found path to goal!
			break;

		if (current->timestep > 500)
		{
			std::cout << "A* search timed out: took too long!" << std::endl;
			break;
		}

#if DEBUG_VERBOSE
		std::cout << "A* CHOSE TO EXPAND: " << *current->tile << " at Time " << current->timestep << " with Estimate " << current->estimate << std::endl;
#endif

		open.pop();

		//current->color = vec3(0, 0, 1);

		ExpandNeighbor(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, 0, 1), start, goal, customCosts);
		ExpandNeighbor(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, 1, 0), start, goal, customCosts);
		ExpandNeighbor(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, 0, -1), start, goal, customCosts);
		ExpandNeighbor(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, -1, 0), start, goal, customCosts);
		ExpandNeighbor(open, current, currentTile, gridMap->getTileRelativeTo(current->tile, 0, 0), start, goal, customCosts);
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
	Stats::avgSearchTime = timer.GetAvgTime();

	std::cout << "Search visited: " << modifiedTiles.size() << " tiles | Expanded: " << NODES_EXPANDED << " tiles. Took " << timer.GetTimeElapsed() << " seconds" << std::endl;

	sortTimer.PrintTimeAccumulated("Sort Timer");
	sortTimer.Reset();

	freeTileInfos.splice(freeTileInfos.end(), usedTileInfos);

	// reset visited tiles
	for (Tile* tile : modifiedTiles) tile->Reset();

	return path;
}

void TemporalAStar::ExpandNeighbor(OpenQueue& open, TileTime* currentInfo, Tile* currentTile, Tile* neighborTile, Tile* start, Tile* goal, TileCosts& customCosts)
{
	int timestep = currentInfo->timestep + 1;
	bool isWaitAction = currentInfo->tile == neighborTile;

	float actionCost = isWaitAction ? 0.75f : 1.f;
	float newCost = currentInfo->cost + actionCost;

	Tile* from = currentInfo->tile;

	if (neighborTile && neighborTile->isWalkable && !neighborTile->visitedAtTime[currentTile][timestep])
	{
#if DEBUG_VERBOSE
		std::cout << *neighborTile << " at " << " time: " << timestep << std::endl;
#endif
		modifiedTiles.push_back(neighborTile);

		neighborTile->numberOfTimesVisited += 1;

		neighborTile->visitedAtTime[currentTile][timestep] = true;

		//std::cout << *from << " " << *tile << " " << timestep << std::endl;

		if (!customCosts.empty())
		{
			bool hasCustomCost = customCosts.count(timestep) && customCosts[timestep].count(neighborTile);
			if (hasCustomCost)
				newCost += customCosts[timestep][neighborTile];

#if DEBUG_VERBOSE
			if (hasCustomCost)
				std::cout << "Using cost!" << std::endl;
#endif
		}

		TileTime* newTileInfo = nullptr;
		if (!freeTileInfos.empty())
		{
			newTileInfo = freeTileInfos.back();
			freeTileInfos.pop_back();
		}
		else
		{
			newTileInfo = new TileTime();
		}

		newTileInfo->SetInfo(timestep, neighborTile, newCost, neighborTile->CalculateEstimate(newCost, goal));
		usedTileInfos.push_back(newTileInfo);
		cameFrom[newTileInfo] = currentInfo;

		open.push(newTileInfo);
		//tile->color = vec3(0, 1, 1);
	}
}

bool BaseHeuristic::operator()(TileTime* A, TileTime* B)
{
	return A->estimate > B->estimate;
}


//////////////////////////////////////////////////////////////////////////
// TESTING
//////////////////////////////////////////////////////////////////////////

MAPF::Path TemporalAStar::FindPath2(Tile* start, Tile* goal, TileCosts& customCosts)
{
	timer.Begin();
	MAPF::Path path;
	OpenQueue2 open2;

	modifiedTileTimes.clear();

	TileTime2* initial = new TileTime2();
	initial->SetInfo(1, start, 0, start->CalculateEstimate(0, goal));
	initial->bIsInOpen = true;
	open2.push(initial);
	modifiedTileTimes.emplace(initial);

	TileTime2* current = nullptr;
	
	while (!open2.empty())
	{
		current = open2.top();
		current->bClosed = true;
		
#if DEBUG_VERBOSE
		std::cout << "Expanding " << *current->tile << " at time " << current->timestep << std::endl;
#endif

		open2.pop();

		if (current->tile == goal) // found path to goal!
			break;

		ExpandNeighbor2(open2, current, current->tile, start, goal, customCosts);
		ExpandNeighbor2(open2, current, gridMap->getTileRelativeTo(current->tile, 0, 1), start, goal, customCosts);
		ExpandNeighbor2(open2, current, gridMap->getTileRelativeTo(current->tile, 1, 0), start, goal, customCosts);
		ExpandNeighbor2(open2, current, gridMap->getTileRelativeTo(current->tile, 0, -1), start, goal, customCosts);
		ExpandNeighbor2(open2, current, gridMap->getTileRelativeTo(current->tile, -1, 0), start, goal, customCosts);
	}

	// build the path
	while (current != nullptr)
	{
		if (current->countFrom.find(current->parent) == current->countFrom.end())
			current->countFrom[current->parent] = 0;

		current->countFrom[current->parent] += 1;
		path.push_front(current->tile);
		current = current->parent;
	}

	// reset any modified tiles
	for (TileTime2* modified : modifiedTileTimes)
		modified->Reset();

	timer.End();
	Stats::avgSearchTime = timer.GetAvgTime();

	return path;
}

void TemporalAStar::ExpandNeighbor2(OpenQueue2& open, TileTime2* current, Tile* neighborTile, Tile* start, Tile* goal, TileCosts& customCosts)
{
	if (!neighborTile || !neighborTile->isWalkable) return;

	// get neighbor tile timestep and new cost
	int neighborTimestep = current->timestep + 1;

	// try to get the tile info from the spatial grid map
	TileTime2* neighbor;
	if (spatialGridMap[neighborTile][neighborTimestep] != nullptr)
	{
		neighbor = spatialGridMap[neighborTile][neighborTimestep];
	}
	else // otherwise create a new info and add it to the grid map
	{
		neighbor = new TileTime2();
		spatialGridMap[neighborTile][neighborTimestep] = neighbor;
	}
	
	modifiedTileTimes.emplace(neighbor);

	if (neighbor->bClosed) // don't do anything if the node has already been expanded
		return;

	float cost = current->cost + 1;

	float customCost = GetCustomCosts(current->timestep, neighborTile, customCosts);
	if (customCost > 0) 
		std::cout << "\t\t\t\tUSING CUSTOM COST ON TILE " << *neighborTile << " ON TIME " << current->timestep << " VALUE " << customCost << std::endl;
	cost += customCost;

	if (neighbor->bIsInOpen && !neighbor->bNeedsReset) // relax the node - update the parent
	{
		float parentCost = neighbor->parent->cost;
		
		//std::cout << "Comparing " << parentCost << " and " << cost << std::endl;
		
		if (current->cost == parentCost)
		{
#if DEBUG_VERBOSE
			std::cout << "COSTS ARE THE SAME NOW COMPARE # CUSTOM COSTS: Current " << 
				*current->tile << " | " << GetCustomCosts(current->timestep, neighborTile, customCosts) << " Old " << 
				*neighbor->parent->tile << " | " << GetCustomCosts(current->timestep, neighbor->tile, customCosts) << std::endl;
#endif
			
			if (neighbor->countFrom[current] < neighbor->countFrom[neighbor->parent])
			{
#if DEBUG_VERBOSE
				std::cout << "Changed parent as this path was used previously" << std::endl;
#endif
				neighbor->SetParent(current);
			}
		}
		else if (current->cost < parentCost)
		{
#if DEBUG_VERBOSE
			std::cout << "Changed parent!" << std::endl;
#endif
			neighbor->SetParent(current);
		}
	}
	else // create a new info and add it to the open queue
	{
		float estimate = neighborTile->CalculateEstimate(cost, goal);
		neighbor->bIsInOpen = true;
		neighbor->SetInfo(neighborTimestep, neighborTile, cost, estimate);
		neighbor->SetParent(current);
		neighbor->bNeedsReset = false;

#if DEBUG_VERBOSE
		std::cout << "Added " << *neighborTile << 
			" to open list with cost " <<  cost << 
			" and heur " << neighborTile->heuristic <<
			" and est " << estimate << 
			" at " << neighborTimestep << std::endl;
#endif

		open.push(neighbor);
	}
}

int TemporalAStar::GetCustomCosts(int timestep, Tile* tile, TileCosts& customCosts)
{
	bool hasCustomCost = customCosts.count(timestep) && customCosts[timestep].count(tile);
	return hasCustomCost ? customCosts[timestep][tile] : 0;
}

bool BaseHeuristic2::operator()(TileTime2* A, TileTime2* B)
{
	return A->estimate > B->estimate;
}
