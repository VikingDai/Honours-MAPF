#include "TemporalBFS.h"

TemporalBFS::TemporalBFS(GridMap* inGridMap)
{
	gridMap = inGridMap;
}

MAPF::Path TemporalBFS::FindNextPath(Tile* start, Tile* goal)
{
	nodesExpanded = 0;

	MAPF::Path path;

	if (!start || !goal)
		return path;

	if (frontier.empty())
		frontier.push(new TileTime(start, 0));

	std::vector<TileTime*> tilesAtGoal;

	TileTime* current = nullptr;

	while (!frontier.empty())
	{
		current = frontier.front();
		frontier.pop();

		nodesExpanded += 1;

		//std::cout << *current->first << " | " << current->second << std::endl;

		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, 0, 1));
		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, 1, 0));
		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, 0, -1));
		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, -1, 0));
		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, 0, 0));

		if (current->first == goal)
			break;
	}

	// rebuild paths
	while (cameFrom.find(current) != cameFrom.end())
	{
		path.push_front(current->first);
		current = cameFrom[current];
	}

	// print stats
	std::cout << "Expanded " << nodesExpanded << " tiles" << std::endl;

	return path;
}

std::vector<MAPF::Path> TemporalBFS::SearchToDepth(Tile* start, Tile* goal, int depth)
{
	nodesExpanded = 0;

	std::vector<MAPF::Path> paths;

	if (!start || !goal || depth <= 0) 
		return paths;

	frontier.push(new TileTime(start, 0));

	std::vector<TileTime*> tilesAtGoal;

	while (!frontier.empty())
	{
		TileTime* current = frontier.front();
		frontier.pop();

		nodesExpanded += 1;

		//std::cout << *current->first << " | " << current->second << std::endl;

		if (current->first == goal && current->second == depth)
			tilesAtGoal.push_back(current);

		if (current->second > depth) break;

		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, 0, 1));
		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, 1, 0));
		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, 0, -1));
		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, -1, 0));
		AddNeighbor(current, gridMap->getTileRelativeTo(current->first, 0, 0));
	}

	// rebuild paths
	for (TileTime* tileTime : tilesAtGoal)
	{
		MAPF::Path path;
		TileTime* current = tileTime;

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

void TemporalBFS::AddNeighbor(TileTime* current, Tile* neighbor)
{
	if (!current || !neighbor) return;

	if (neighbor->isWalkable)
	{
		//std::pair<Tile*, int> test;
		TileTime* neighborTileTime = new TileTime(neighbor, current->second + 1);
		frontier.push(neighborTileTime);
		cameFrom[neighborTileTime] = current;
	}
}
