#include "Agent.h"
#include "SpatialAStar.h"
#include "Tile.h"
#include "GridMap.h"
#include "MathUtils.h"
#include "Graphics.h"

static int agentCount = 0;

Agent::Agent(GridMap* gridMap, Tile* startTile, Tile* goalTile) : EObject(startTile->x, startTile->y)
{
	assert(startTile);

	color = vec3(0, MathUtils::RandomFloat(), MathUtils::RandomFloat());

	agentId = agentCount;
	agentCount += 1;

	renderPos = vec3(startTile->x, startTile->y, 0);
	
	goal = goalTile;

	bfs = new SpatialBFS(gridMap);
}

void Agent::step()
{
	if (!currentPath.empty())
	{
		// move along current path assigned to us by the agent coordinator
		Tile* nextTile = currentPath.front();
		currentPath.pop_front();

		x = nextTile->x;
		y = nextTile->y;

		std::vector<SpatialAStar::Path> pathsToRemove;
		for (SpatialAStar::Path& allPath : allPaths)
		{
			allPath.pop_front();
			if (allPath.empty())
				pathsToRemove.push_back(allPath);
		}

		for (SpatialAStar::Path& pathToRemove : pathsToRemove)
		{
			auto it = std::find(allPaths.begin(), allPaths.end(), pathToRemove);
			if (it != allPaths.end())
				allPaths.erase(it);
		}

		for (SpatialAStar::Path& allPath : allPaths)
			assert(!allPath.empty());
	}
	
	
	if (currentPath.empty()) // we have reached our goal
		goal = nullptr;
}

void Agent::setPath(SpatialAStar::Path& inPath)
{
	currentPath = inPath;
}

void Agent::update(float dt)
{
	renderPos.x += (x - renderPos.x) * dt * 10.f;
	renderPos.y += (y - renderPos.y) * dt * 10.f;
}

void Agent::drawPaths(Graphics* graphics)
{
	std::vector<vec3> points;

	for (int i = 0; i < allPaths.size(); i++) //AStar::Path& path : allPaths)
	{
		SpatialAStar::Path& path = allPaths[i];
		float pathSep = .4 / allPaths.size();
		for (Tile* tile : path)
		{
			points.emplace_back(vec3(tile->x + pathSep * i - 0.2f, tile->y + pathSep * i - 0.2f, 0));
		}
		float thickness = currentPath == path ? 5.f : 2.f;
		graphics->DrawLine(points, color, thickness);
		points.clear();
	}
}

std::ostream& operator<<(std::ostream& os, Agent& agent)
{
	os << "Agent " << agent.getAgentId() << "[" << agent.x << "," << agent.y << " | Goal " << *agent.goal << "]";
	return os;
}
