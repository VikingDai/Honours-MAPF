#include "Agent.h"
#include "AStar.h"
#include "Tile.h"
#include "GridMap.h"
#include "MathUtils.h"

static int agentCount = 0;

Agent::Agent(int x, int y) : EObject(x, y)
{
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
		Tile* nextTile = path.front();
		path.pop_front();

		x = nextTile->x;
		y = nextTile->y;

		std::vector<AStar::Path> pathsToRemove;
		for (AStar::Path& allPath : allPaths)
		{
			allPath.pop_front();
			if (allPath.empty())
				pathsToRemove.push_back(allPath);
		}

		for (AStar::Path& pathToRemove : pathsToRemove)
		{
			auto it = std::find(allPaths.begin(), allPaths.end(), pathToRemove);
			if (it != allPaths.end())
				allPaths.erase(it);
		}

		for (AStar::Path& allPath : allPaths)
			assert(!allPath.empty());
	}
}

void Agent::setPath(AStar::Path& inPath)
{
	path = inPath;
}

void Agent::update(float dt)
{
	renderPos.x += (x - renderPos.x) * dt * 10.f;
	renderPos.y += (y - renderPos.y) * dt * 10.f;
}

std::ostream& operator<<(std::ostream& os, Agent& agent)
{
	os << "Agent " << agent.getAgentId() << ": (" << agent.x << "," << agent.y << ")";
	return os;
}
