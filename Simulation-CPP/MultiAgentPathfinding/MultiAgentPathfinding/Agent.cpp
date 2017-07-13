#include "Agent.h"
#include "AStar.h"
#include "Tile.h"
#include "GridMap.h"
#include "MathUtils.h"

static int agentCount = 0;

Agent::Agent(int x, int y) : EObject(x, y)
{
	pathIndex = 0;
	hasReachedGoal = false;

	color = vec3(MathUtils::randomFloat() * 0.6, MathUtils::randomFloat() * 0.6, MathUtils::randomFloat()  * 0.6);

	agentId = agentCount;
	agentCount += 1;
}

void Agent::step()
{
	if (!path.empty())
	{
		// move along current path assigned to us by the agent coordinator
		/*Tile* nextTile = path[pathIndex];
		
		pathIndex += 1;*/
		Tile* nextTile = path.front();
		path.pop_front();

		x = nextTile->x;
		y = nextTile->y;
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
