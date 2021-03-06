#include "TemporalAStar.h"
#include "Tile.h"
#include "GridMap.h"
#include <deque>
#include <chrono>
#include <ctime>
#include "Statistics.h"
#include "Heuristics.h"

#define DEBUG_VERBOSE 0
#define DEBUG_STATS 0
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

MAPF::Path TemporalAStar::FindPath(Tile* start, Tile* goal, MAPF::CollisionPenalties& penalties)
{
#if DEBUG_VERBOSE
	std::cout << std::endl << "### FINDING PATH FROM " << *start << " to " << *goal << std::endl;
	std::cout << "PENALTIES: " << std::endl;
	for (auto& it : penalties.edge)
	{
		int time = it.first;
		auto& it2 = it.second;
		for (auto& pair : it2)
		{
			std::cout << "\tACTION FROM " << *pair.first.first << " TO " << *pair.first.second << " at time " << time << " Has a penalty of " << pair.second << std::endl;
		}
	}
	std::cout << std::endl;
#endif

	MAPF::Path path(penalties);

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

#if DEBUG_VERBOSE
		std::cout << "### Choosing from Open ";
		std::cout << open << std::endl;
#endif
		current = open.Pop();
#if DEBUG_SIMPLE
		std::cout << "Temporal A* expanded " << *current << std::endl;
#endif
		
		current->bClosed = true;
		
//#if DEBUG_VERBOSE

		//current->tile->SetColor(sf::Color::Red);

		if (current->tile == goal) // found path to goal!
		{
			path.cost = current->GetG();
			break;
		}

		ExpandNeighbor(open, current, current->tile, start, goal, penalties); // wait
		ExpandNeighbor(open, current, gridMap->GetTileRelativeTo(current->tile, 0, 1), start, goal, penalties); // up
		ExpandNeighbor(open, current, gridMap->GetTileRelativeTo(current->tile, -1, 0), start, goal, penalties); // left
		ExpandNeighbor(open, current, gridMap->GetTileRelativeTo(current->tile, 0, -1), start, goal, penalties); // down
		ExpandNeighbor(open, current, gridMap->GetTileRelativeTo(current->tile, 1, 0), start, goal, penalties); // right
		

#if DEBUG_STATS
		std::cout << "Temporal A* has expanded: " << LOCAL_TILES_EXPANDED << std::endl;
#endif
	}

	// build the path
	while (current != nullptr)
	{
		path.tiles.push_front(current->tile);
		current = current->parent;
	}

#if DEBUG_VERBOSE
	std::cout << "Temporal A* Path:" << std::endl;
	for (Tile* tile : path.tiles)
		std::cout << *tile << " > ";
	std::cout << std::endl;
#endif

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

	for (Tile* tile : path.tiles)
		tile->SetColor(sf::Color::Blue);

	return path;
}

void TemporalAStar::ExpandNeighbor(OpenQueue& open, AStarTileTime* current, Tile* neighborTile, Tile* start, Tile* goal, MAPF::CollisionPenalties& penalties)
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

	if (neighbor->bClosed) // skip if the node has already been expanded
		return;

	LOCAL_TILES_EXPANDED += 1;
	neighborTile->SetColor(sf::Color(current->tile->GetColor().r, current->tile->GetColor().g + 20, current->tile->GetColor().b, current->tile->GetColor().a));

	float penalty = GetCustomCosts(neighborTimestep, current->tile, neighborTile, penalties);

#if 0
	if (penalty > 0) 
		std::cout << "\tUsed CUSTOM COST for action " << *current->tile << " to " << *neighborTile << " at time " << current->timestep << " with value " << penalty << std::endl;
#endif
	
	if (neighbor->bIsInOpen) // relax the node - update the parent
	{
#if DEBUG_VERBOSE
		std::cout << *neighbor << " is already in open, updating it"<< std::endl;
#endif

		float newNeighborCost = current->GetG() + penalty;
		float neighborCost = neighbor->GetG();
		
		if (newNeighborCost == neighborCost)
		{
			float currentPenalty = current->GetPenalty();
			bool newIsBetter = currentPenalty < penalty;
			if (newIsBetter)
			{
#if DEBUG_VERBOSE
				std::cout << "penalty lower: accepting new neighbor" << std::endl;
#endif
				neighbor->SetParent(current);
				neighbor->SetPenalty(penalty);
				neighbor->UpdateCosts();
			}
		}
		else if (newNeighborCost < neighborCost)
		{
#if DEBUG_VERBOSE
			std::cout << "g is lower: accepting new neighbor" << std::endl;
#endif
			neighbor->SetParent(current);
			neighbor->SetPenalty(penalty);
			neighbor->UpdateCosts();
		}
#if DEBUG_VERBOSE
		std::cout << "\tUPDATED " << *neighbor << std::endl;
#endif
	}
	else // create a new info and add it to the open queue
	{
		modifiedTiles.push_back(neighborTile);

		float heuristic = Heuristics::Manhattan(neighborTile, goal);
		neighbor->bIsInOpen = true;
		neighbor->SetInfo(current, neighborTimestep, neighborTile, heuristic, penalty);

#if DEBUG_VERBOSE
		std::cout << "\tGenerated " << *neighbor << std::endl;
#endif

		open.Push(neighbor);
	}
}

int TemporalAStar::GetCustomCosts(int timestep, Tile* fromTile, Tile* toTile, MAPF::CollisionPenalties& penalties)
{
	std::pair<Tile*, Tile*> action(fromTile, toTile);
	bool hasActionPenalty = penalties.edge.count(timestep) && penalties.edge[timestep].count(action);
	auto& actionMap = penalties.edge[timestep];
	float actionPenalty = actionMap.count(action) ? actionMap[action] : 0;

	return actionPenalty;
}