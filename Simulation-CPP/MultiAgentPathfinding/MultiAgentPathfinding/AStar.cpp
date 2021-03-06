#include "AStar.h"
#include <algorithm>
#include <stack>
#include "PriorityQueue.h"

#define DEBUG_ASTAR 0
#define DEBUG_LOG 0
#define DEBUG_STATS 0

AStar::AStar(GridMap* inGridMap)
{
	gridMap = inGridMap;
}

MAPF::Path AStar::FindPath(Tile* start, Tile* goal)
{
	timer.Begin();
	MAPF::Path path;

	if (!start || !goal || start == goal || !start->isWalkable || !goal->isWalkable)
	{
		// no path was found, return a path of length 1 where the agent does not move
		path.tiles.push_front(start);
		return path;
	}

	LOCAL_EXP = 0;
	LOCAL_GEN = 0;
	LOCAL_TOUCH = 0;

	for (Tile* tile : modifiedTiles)
		tile->ResetColor();
	modifiedTiles.clear();
	OpenQueue open(1024, true);
	//OpenQueue open;

	start->CalculateEstimate(0, goal);

	//open.push_back(start);
	//open.push(start);
	open.Push(start);

	modifiedTiles.push_back(start);

	Tile* current = start;
	while (!open.Empty())
	{
		/*std::sort(open.begin(), open.end(), Heuristic());
		current = open.back();
		open.pop_back();*/

		
		current = open.Pop();

		//current = open.top();
		//open.pop();

#if DEBUG_ASTAR
		std::cout << "### Choosing from Open ";
		std::cout << open << std::endl;
		std::cout << "Expanded: " << *current << " with estimate: " << current->f << " cost: " << current->g << std::endl;
#endif

		//std::cout << open << std::endl;
		//std::cout << "--------------------------" << std::endl << std::endl;

		LOCAL_EXP += 1;


		

		current->SetColor(sf::Color::Red);

		if (current == goal) // found path to the goal
		{
			path.cost = current->g;
			break;
		}

		current->hasBeenExpanded = true;

		// add the neighbors of the current tile (up, down, left, right) to the open list
		AddNeighbor(open, current, gridMap->GetTileRelativeTo(current, 0, 1), start, goal); // up
		AddNeighbor(open, current, gridMap->GetTileRelativeTo(current, -1, 0), start, goal); // left
		AddNeighbor(open, current, gridMap->GetTileRelativeTo(current, 0, -1), start, goal); // down
		AddNeighbor(open, current, gridMap->GetTileRelativeTo(current, 1, 0), start, goal); // right
		
	}

	// build the path
	while (current != nullptr)
	{
		path.tiles.push_front(current);
		current = current->parent;
	}

	// reset all modified tiles
	for (Tile* tile : modifiedTiles) 
		tile->Reset();

	timer.End();

#if DEBUG_STATS
	std::cout << "AStar exp <" << LOCAL_EXP << "> | AStar gen <" << LOCAL_GEN << "> | AStar touch <" << LOCAL_TOUCH << "> | Took " << timer.GetTimeElapsed() << " | " << timer.GetTimeElapsed() / LOCAL_GEN << " per expansion  | found path size " << path.size() << std::endl;
#endif

	for (Tile* tile : path.tiles)
		tile->SetColor(sf::Color::Blue);

	

	return path;
}

void AStar::AddNeighbor(OpenQueue& open, Tile* current, Tile* neighbor, Tile* start, Tile* goal)
{
	assert(current && start && goal);

	if (!neighbor || !neighbor->isWalkable) return; 
	// don't expand obstacles
	// this will occur when a tile is out of bounds

	LOCAL_TOUCH += 1;

	// skip neighbors which have already been expanded
	if (neighbor->hasBeenExpanded) 
		return;

#if DEBUG_LOG
	std::cout << "\tSuccessor: " << *neighbor << std::endl;
#endif

	float newNeighborCost = current->g + 1;

	if (neighbor->isInOpen)
	{
#if DEBUG_ASTAR
		std::cout << "UPDATED " << *neighbor << " in open list." << std::endl;
#endif

		// utils/pqueue
		// utils/heap.h
		// ch branch
		// lazy clearing - use search id - check id and status (search_node)

		// update neighbor in the fringe by checking if this new parent tile is better
		// 
		bool newCostIsBetter = newNeighborCost < neighbor->g;
		if (newCostIsBetter) // only accept this new path if the cost is less
		{
#if DEBUG_LOG
			std::cout << "\t\tUpdating with new cost for " << *neighbor << std::endl;
#endif
			neighbor->CalculateEstimate(newNeighborCost, goal);
			neighbor->parent = current;
			open.HeapifyUp(neighbor->GetPriority());
		}
	}
	else // add neighbor to the fringe
	{
		LOCAL_GEN += 1;

		neighbor->isInOpen = true;
		neighbor->CalculateEstimate(newNeighborCost, goal);
		neighbor->parent = current;

		//open.push_back(neighbor);
		//open.push(neighbor);
		open.Push(neighbor);

#if 0
		std::cout << "\tADDED " << *neighbor << " to open list, f: " << neighbor->f << std::endl;
#endif

		neighbor->SetColor(sf::Color::Green);

		modifiedTiles.push_back(neighbor);
	}
}