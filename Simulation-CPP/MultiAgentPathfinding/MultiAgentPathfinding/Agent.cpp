#include "Agent.h"
#include "TemporalAStar.h"
#include "Tile.h"
#include "GridMap.h"
#include "MathUtils.h"
#include "Graphics.h"

int Agent::agentCounter = 0;

Agent::Agent(GridMap* gridMap, Tile* startTile, Tile* goalTile) : EObject(startTile->x, startTile->y)
{
	assert(startTile);

	color = vec3(0, MathUtils::RandomFloat(), MathUtils::RandomFloat());

	agentId = agentCounter;
	agentCounter += 1;

	renderPos = vec3(startTile->x, startTile->y, 0);
	
	goal = goalTile;

	bfs = new TemporalBFS(gridMap);

	temporalAStar = new TemporalAStar(gridMap);

	aStar = new AStar(gridMap);
}

void Agent::step()
{
	if (!chosenPath.empty())
	{
		// move along current path assigned to us by the agent coordinator
		Tile* nextTile = chosenPath.front();
		chosenPath.pop_front();

		x = nextTile->x;
		y = nextTile->y;

		std::vector<TemporalAStar::Path> pathsToRemove;
		for (TemporalAStar::Path& allPath : potentialPaths)
		{
			allPath.pop_front();
			if (allPath.empty())
				pathsToRemove.push_back(allPath);
		}

		for (TemporalAStar::Path& pathToRemove : pathsToRemove)
		{
			auto it = std::find(potentialPaths.begin(), potentialPaths.end(), pathToRemove);
			if (it != potentialPaths.end())
				potentialPaths.erase(it);
		}

		for (TemporalAStar::Path& allPath : potentialPaths)
			assert(!allPath.empty());
	}
	
	
	if (chosenPath.empty()) // we have reached our goal
		goal = nullptr;
}

void Agent::setPath(TemporalAStar::Path& inPath)
{
	chosenPath = inPath;
}

void Agent::update(float dt)
{
	renderPos.x += (x - renderPos.x) * dt * 10.f;
	renderPos.y += (y - renderPos.y) * dt * 10.f;
}

void Agent::drawPaths(Graphics* graphics)
{
	std::vector<vec3> points;

	for (int i = 0; i < potentialPaths.size(); i++) //AStar::Path& path : allPaths)
	{
		TemporalAStar::Path& path = potentialPaths[i];
		float pathSep = .4 / potentialPaths.size();
		for (Tile* tile : path)
		{
			points.emplace_back(vec3(tile->x + pathSep * i - 0.2f, tile->y + pathSep * i - 0.2f, 0));
		}
		float thickness = chosenPath == path ? 5.f : 2.f;
		graphics->DrawLine(points, color, thickness);
		points.clear();
	}
}

void Agent::drawLineToGoal(Graphics* graphics)
{
	if (!goal) return;

	std::vector<vec3> points;
	points.emplace_back(vec3(x, y, 0));
	points.emplace_back(vec3(goal->x, goal->y, 0));
	graphics->DrawLine(points, color, 1.f);
}

std::ostream& operator<<(std::ostream& os, Agent& agent)
{
	os << "Agent " << agent.getAgentId() << "[" << agent.x << "," << agent.y << " | Goal " << *agent.goal << "]";
	return os;
}
