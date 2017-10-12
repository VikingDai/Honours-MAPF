#include "TemporalAStar.h"
#include "Tile.h"
#include "GridMap.h"
#include <deque>
#include <chrono>
#include <ctime>
#include "Statistics.h"
#include "Heuristics.h"

#define DEBUG_VERBOSE 0
#define DEBUG_STATS 1

#define DEBUG_SIMPLE 0

int TemporalAStar::GLOBAL_TILES_EXPANDED = 0;

std::vector<AStarTileTime*> AStarTileTime::TILE_TIME_POOL;

AStarTileTime* AStarTileTime::Make(std::vector<AStarTileTime*>& usedTileTimes)
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

MAPF::Path TemporalAStar::FindPath(Tile* start, Tile* goal, CollisionPenalties& penalties)
{
	MAPF::Path path;

	if (!start || !goal || start == goal)
		return path;

	LOCAL_TILES_EXPANDED = 0;

	for (Tile* tile : modifiedTiles)
		tile->ResetColor();
	modifiedTiles.clear();

	timer.Begin();
	
	OpenQueue open(1024, true);

	AStarTileTime* initial = AStarTileTime::Make(usedTileTimes);
	
	initial->SetInfo(nullptr, 0, start, Heuristics::Manhattan(start, goal), 0);
	initial->bIsInOpen = true;

	open.Push(initial);

	modifiedTileTimes.push_back(initial);

	AStarTileTime* current = nullptr;
	
	while (!open.Empty())
	{	
		//std::cout << ">>>>> BEGIN OPEN QUEUE"<< std::endl;
		//std::cout << open << std::endl;
		//std::cout << "<<<<< END OPEN QUEUE" << std::endl;

		current = open.Pop();
		
		current->bClosed = true;
		
//#if DEBUG_VERBOSE
#if DEBUG_SIMPLE
		std::cout << "EXPAND " << *current << std::endl;
#endif
		//current->tile->SetColor(sf::Color::Red);

		if (current->tile == goal) // found path to goal!
			break;

		ExpandNeighbor(open, current, gridMap->GetTileRelativeTo(current->tile, 0, 1), start, goal, penalties); // up
		ExpandNeighbor(open, current, gridMap->GetTileRelativeTo(current->tile, -1, 0), start, goal, penalties); // left
		ExpandNeighbor(open, current, gridMap->GetTileRelativeTo(current->tile, 0, -1), start, goal, penalties); // down
		ExpandNeighbor(open, current, gridMap->GetTileRelativeTo(current->tile, 1, 0), start, goal, penalties); // right
		ExpandNeighbor(open, current, current->tile, start, goal, penalties); // wait
		

#if DEBUG_VERBOSE
		std::cout << "Temporal A* has expanded: " << LOCAL_TILES_EXPANDED << std::endl;
#endif
	}

	// build the path
	while (current->parent != nullptr)
	{
		path.push_front(current->tile);
		current = current->parent;
	}

	// reset any modified tiles
	for (AStarTileTime* modified : modifiedTileTimes)
		modified->Reset();

	modifiedTileTimes.clear();

#if DEBUG_STATS
	timer.End();
	Stats::avgSearchTime = timer.GetAvgTime();
	std::cout << "Temporal AStar expanded <" << LOCAL_TILES_EXPANDED << 
		"> tiles | Took " << timer.GetTimeElapsed() << " | " << 
		timer.GetTimeElapsed() / LOCAL_TILES_EXPANDED << 
		" per expansion | found path size " << path.size() << 
		" | sorting " << sortTimer.GetTimeAccumulated() << std::endl;
#endif

	GLOBAL_TILES_EXPANDED += LOCAL_TILES_EXPANDED;

	for (AStarTileTime* tileTime : usedTileTimes)
		tileTime->Reset();
	AStarTileTime::TILE_TIME_POOL.insert(AStarTileTime::TILE_TIME_POOL.end(), usedTileTimes.begin(), usedTileTimes.end());
	usedTileTimes.clear();

	spatialGridMap.clear();

	closed.clear();

	sortTimer.Reset();

	for (Tile* tile : path)
		tile->SetColor(sf::Color::Blue);

	return path;
}

void TemporalAStar::ExpandNeighbor(OpenQueue& open, AStarTileTime* current, Tile* neighborTile, Tile* start, Tile* goal, CollisionPenalties& penalties)
{
	if (!neighborTile || !neighborTile->isWalkable) return;

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
		neighbor = AStarTileTime::Make(usedTileTimes);
		spatialGridMap[neighborTile][neighborTimestep] = neighbor;
		modifiedTileTimes.push_back(neighbor);
	}

	//if (std::find(closed.begin(), closed.end(), neighbor) != closed.end())
	//if (!spatialGridMap[neighborTile].empty())
		//return;

	if (neighbor->bClosed) // skip if the node has already been expanded
		return;

	LOCAL_TILES_EXPANDED += 1;
	neighborTile->SetColor(sf::Color(current->tile->GetColor().r, current->tile->GetColor().g + 20, current->tile->GetColor().b, current->tile->GetColor().a));

	float customCost = GetCustomCosts(current->timestep, current->tile, neighborTile, penalties);
	float customPrev = GetCustomCosts(current->timestep - 1, current->tile, neighborTile, penalties);
	float cost = current->g + 1;

#if 0
	if (customCost > 0) 
		std::cout << "\t\t\t\tUSING CUSTOM COST ON TILE " << *neighborTile << " ON TIME " << current->timestep << " VALUE " << customCost << std::endl;
#endif
	
	if (neighbor->bIsInOpen) // relax the node - update the parent
	{
		float parentCost = neighbor->parent->g;
		
		if (current->g == parentCost)
		{
			float currentParentCustom = current->penalty;
			bool newIsBetter = currentParentCustom < customCost;
			if (newIsBetter)
			{
				neighbor->SetParent(current);
				neighbor->UpdateCosts();
			}
		}
		else if (current->g < parentCost)
		{
			neighbor->SetParent(current);
			neighbor->UpdateCosts();
		}
	}
	else // create a new info and add it to the open queue
	{
		modifiedTiles.push_back(neighborTile);

		float heuristic = Heuristics::Manhattan(neighborTile, goal);
		neighbor->bIsInOpen = true;
		neighbor->SetInfo(current, neighborTimestep, neighborTile, heuristic, customCost);

#if DEBUG_SIMPLE
		std::cout << "\tADDED " << *neighbor << std::endl;
#endif

		open.Push(neighbor);
	}
}

int TemporalAStar::GetCustomCosts(int timestep, Tile* fromTile, Tile* toTile, CollisionPenalties& penalties)
{
	std::pair<Tile*, Tile*> action(fromTile, toTile);
	bool hasActionPenalty = penalties.actionCollisions.count(timestep) && penalties.actionCollisions[timestep].count(action);
	auto& actionMap = penalties.actionCollisions[timestep];
	float actionPenalty = actionMap.count(action) ? actionMap[action] : 0;

	return actionPenalty;
	//bool hasCustomCost = penalties.tileCollisions.count(timestep) && penalties.tileCollisions[timestep].count(fromTile);
	//return hasCustomCost ? penalties.tileCollisions[timestep][fromTile] : 0;
}