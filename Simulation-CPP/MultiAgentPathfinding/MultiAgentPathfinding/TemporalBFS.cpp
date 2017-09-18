#include "TemporalBFS.h"

#define DEBUG_VERBOSE 1

using BFSTileTime = std::pair<Tile*, int>;

int TemporalBFS::GLOBAL_TILES_EXPANDED = 0;

std::vector<BFSTileTime*> TemporalBFS::TILE_TIME_POOL;

TemporalBFS::BFSTileTime* TemporalBFS::MakeTileTime(std::vector<BFSTileTime*>& usedTileTimes, Tile* tile, int time)
{
	BFSTileTime* tileTime = nullptr;

	if (TILE_TIME_POOL.empty())
	{
		tileTime = new BFSTileTime(tile, time);
	}
	else
	{
		tileTime = TILE_TIME_POOL.back();
		tileTime->first = tile;
		tileTime->second = time;
		TILE_TIME_POOL.pop_back();
	}

	usedTileTimes.push_back(tileTime);

	return tileTime;
}

TemporalBFS::TemporalBFS(GridMap* inGridMap)
{
	gridMap = inGridMap;
}

void TemporalBFS::Reset()
{
	frontier.swap(std::queue<BFSTileTime*>());
	cameFrom.clear();

	TILE_TIME_POOL.insert(TILE_TIME_POOL.end(), usedTileTimes.begin(), usedTileTimes.end());
	usedTileTimes.clear();
}

MAPF::Path TemporalBFS::FindNextPath(Tile* start, Tile* goal)
{
	nodesExpanded = 0;

	MAPF::Path path;

	if (!start || !goal || start == goal)
		return path;

	if (frontier.empty())
		frontier.push(MakeTileTime(usedTileTimes, start, 0));

	std::vector<BFSTileTime*> tilesAtGoal;

	BFSTileTime* current = nullptr;

	while (!frontier.empty())
	{
		current = frontier.front();
		frontier.pop();

		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, 0, 1));
		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, 1, 0));
		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, 0, -1));
		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, -1, 0));
		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, 0, 0));

		if (current->first == goal)
			break;
	}

	// rebuild paths
	while (cameFrom.find(current) != cameFrom.end())
	{
		path.push_front(current->first);
		//std::cout << "Rebuilding Path: " << *current->first << "(" << current->second << ")" << " > ";

		current = cameFrom[current];
		//std::cout << *current->first << "(" << current->second << ")" << std::endl;
	}

	// print stats
	std::cout << "Exp " << nodesExpanded << " tiles" << "| Path " << path.size() << std::endl;

	return path;
}

void TemporalBFS::AddNeighbor(BFSTileTime* current, Tile* neighbor)
{
	if (!current || !neighbor) return;

	if (!neighbor->isWalkable) return;

	nodesExpanded += 1;
	
	BFSTileTime* neighborTileTime = MakeTileTime(usedTileTimes, neighbor, current->second + 1);
	frontier.push(neighborTileTime);
	cameFrom[neighborTileTime] = current;

	//std::cout << "Setting parent: " << *current->first << "(" << current->second << ")" << " > " <<
	//	*neighborTileTime->first << "(" << neighborTileTime->second << ")" << std::endl;
}

std::vector<MAPF::Path> TemporalBFS::SearchToDepth(Tile* start, Tile* goal, int depth)
{
	nodesExpanded = 0;

	std::vector<MAPF::Path> paths;

	if (!start || !goal || depth <= 0)
		return paths;

	frontier.push(MakeTileTime(usedTileTimes, start, 0));

	std::vector<BFSTileTime*> tilesAtGoal;

	while (!frontier.empty())
	{
		BFSTileTime* current = frontier.front();
		frontier.pop();

		if (current->first == goal && current->second == depth)
			tilesAtGoal.push_back(current);

		if (current->second > depth) break;

		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, 0, 1));
		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, 1, 0));
		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, 0, -1));
		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, -1, 0));
		AddNeighbor(current, gridMap->GetTileRelativeTo(current->first, 0, 0));
	}

	// rebuild paths
	for (BFSTileTime* tileTime : tilesAtGoal)
	{
		MAPF::Path path;
		BFSTileTime* current = tileTime;

		while (cameFrom.find(current) != cameFrom.end())
		{
			path.push_front(current->first);
			current = cameFrom[current];
		}

		paths.push_back(path);
	}

	// print stats
	std::cout << "Expanded " << nodesExpanded << " tiles" << std::endl;

	return paths;
}