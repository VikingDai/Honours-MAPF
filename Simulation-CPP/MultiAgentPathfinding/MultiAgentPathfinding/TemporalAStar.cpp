#include "TemporalAStar.h"
#include "Tile.h"
#include "GridMap.h"
#include <deque>
#include <chrono>
#include <ctime>
#include "Statistics.h"

#define DEBUG_VERBOSE 0
#define DEBUG_STATS 0



std::vector<AStarTileTime*> TemporalAStar::TILE_TIME_POOL;
int TemporalAStar::GLOBAL_TILES_EXPANDED = 0;

AStarTileTime* TemporalAStar::GetTileTime(std::vector<AStarTileTime*>& usedTileTimes)
{
	AStarTileTime* tileTime = nullptr;

	if (TILE_TIME_POOL.empty())
	{
		tileTime = new AStarTileTime();
	}
	else
	{
		tileTime = TILE_TIME_POOL.back();
		TILE_TIME_POOL.pop_back();
	}

	usedTileTimes.push_back(tileTime);

	return tileTime;
}

TemporalAStar::TemporalAStar(GridMap* inGridMap)
{
	SetGridMap(inGridMap);
}

TemporalAStar::~TemporalAStar()
{

}

MAPF::Path TemporalAStar::FindPath(Tile* start, Tile* goal, TileCosts& customCosts)
{
	LOCAL_TILES_EXPANDED = 0;

	timer.Begin();
	MAPF::Path path;
	OpenQueue open2;

	modifiedTileTimes.clear();

	AStarTileTime* initial = GetTileTime(usedTileTimes);
	initial->SetInfo(0, start, 0, start->CalculateEstimate(0, goal));
	initial->bIsInOpen = true;
	open2.push(initial);
	modifiedTileTimes.emplace(initial);

	AStarTileTime* current = nullptr;
	
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

		ExpandNeighbor(open2, current, current->tile, start, goal, customCosts);
		ExpandNeighbor(open2, current, gridMap->GetTileRelativeTo(current->tile, 0, 1), start, goal, customCosts);
		ExpandNeighbor(open2, current, gridMap->GetTileRelativeTo(current->tile, 1, 0), start, goal, customCosts);
		ExpandNeighbor(open2, current, gridMap->GetTileRelativeTo(current->tile, 0, -1), start, goal, customCosts);
		ExpandNeighbor(open2, current, gridMap->GetTileRelativeTo(current->tile, -1, 0), start, goal, customCosts);
	}

	// build the path
	while (current->parent != nullptr)
	{
		if (current->countFrom.find(current->parent) == current->countFrom.end())
			current->countFrom[current->parent] = 0;

		current->countFrom[current->parent] += 1;
		path.push_front(current->tile);
		current = current->parent;
	}

	// reset any modified tiles
	for (AStarTileTime* modified : modifiedTileTimes)
		modified->Reset();

#if DEBUG_STATS
	timer.End();
	Stats::avgSearchTime = timer.GetAvgTime();
	std::cout << "Search finished, expanded <" << LOCAL_TILES_EXPANDED << "> tiles | Took " << std::endl;
#endif

	GLOBAL_TILES_EXPANDED += LOCAL_TILES_EXPANDED;

	for (AStarTileTime* tileTime : usedTileTimes)
		tileTime->Reset();

	TILE_TIME_POOL.insert(TILE_TIME_POOL.end(), usedTileTimes.begin(), usedTileTimes.end());
	usedTileTimes.clear();

	return path;
}

void TemporalAStar::ExpandNeighbor(OpenQueue& open, AStarTileTime* current, Tile* neighborTile, Tile* start, Tile* goal, TileCosts& customCosts)
{
	if (!neighborTile || !neighborTile->isWalkable) return;

	LOCAL_TILES_EXPANDED += 1;

	// get neighbor tile timestep and new cost
	int neighborTimestep = current->timestep + 1;

	// try to get the tile info from the spatial grid map
	AStarTileTime* neighbor;
	if (spatialGridMap[neighborTile][neighborTimestep] != nullptr)
	{
		neighbor = spatialGridMap[neighborTile][neighborTimestep];
	}
	else // otherwise create a new info and add it to the grid map
	{
		neighbor = GetTileTime(usedTileTimes);
		spatialGridMap[neighborTile][neighborTimestep] = neighbor;
	}
	
	modifiedTileTimes.emplace(neighbor);

	if (neighbor->bClosed) // don't do anything if the node has already been expanded
		return;

	float cost = current->cost + 1;

	float customCost = GetCustomCosts(current->timestep, neighborTile, customCosts);
#if DEBUG_VERBOSE
	if (customCost > 0) 
		std::cout << "\t\t\t\tUSING CUSTOM COST ON TILE " << *neighborTile << " ON TIME " << current->timestep << " VALUE " << customCost << std::endl;
#endif
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

bool BaseHeuristic::operator()(AStarTileTime* A, AStarTileTime* B)
{
	return A->estimate > B->estimate;
}
