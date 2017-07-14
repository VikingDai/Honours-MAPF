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

	color = vec3(0, MathUtils::randomFloat(), MathUtils::randomFloat());

	agentId = agentCount;
	agentCount += 1;

	renderPos = vec3(x, y, 0);

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

void Agent::update(float dt)
{
	renderPos.x += (x - renderPos.x) * dt;
	renderPos.y += (y - renderPos.y) * dt;
}

std::ostream& operator<<(std::ostream& os, Agent& agent)
{
	os << "Agent " << agent.getAgentId() << ": (" << agent.x << "," << agent.y << ")";
	return os;
}
