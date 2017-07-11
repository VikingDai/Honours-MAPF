#include "Agent.h"
#include "AStar.h"
#include "Tile.h"
#include "GridMap.h"
#include "MathUtils.h"

Agent::Agent(int x, int y) : EObject(x, y)
{
	pathIndex = 0;
	hasReachedGoal = false;

	color = vec3(MathUtils::randomFloat(), MathUtils::randomFloat(), MathUtils::randomFloat());
}

void Agent::step()
{
	if (pathIndex < path.size())
	{
		// move along current path assigned to us by the agent coordinator
		Tile* nextTile = path[pathIndex];
		x = nextTile->x;
		y = nextTile->y;
		pathIndex += 1;
		//path.pop_front();
	}
	else
	{
		hasReachedGoal = true;
	}
}

void Agent::setPath(std::deque<Tile*> inPath)
{
	path = inPath;
	pathIndex = 0;
	hasReachedGoal = false;
}
