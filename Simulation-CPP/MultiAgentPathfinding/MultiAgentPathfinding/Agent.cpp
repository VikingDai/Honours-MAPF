#include "Agent.h"
#include "TemporalAStar.h"
#include "Tile.h"
#include "GridMap.h"
#include "MathUtils.h"

#include <SFML/Graphics.hpp>

int Agent::agentCounter = 0;

Agent::Agent(GridMap* gridMap, Tile* startTile, Tile* goalTile) : EObject(startTile->x, startTile->y)
{
	assert(startTile);

	color = sf::Color(0, MathUtils::RandomFloat() * 255, MathUtils::RandomFloat() * 255);

	agentId = agentCounter;
	agentCounter += 1;

	renderPos = vec3(startTile->x, startTile->y, 0);

	goal = goalTile;

	bfs = new TemporalBFS(gridMap);

	temporalAStar = new TemporalAStar(gridMap);

	aStar = new AStar(gridMap);
}

void Agent::Step()
{
	if (!chosenPath.empty())
	{
		// move along current path assigned to us by the agent coordinator
		Tile* nextTile = chosenPath.front();
		chosenPath.pop_front();

		x = nextTile->x;
		y = nextTile->y;

		std::vector<MAPF::Path> pathsToRemove;
		for (MAPF::Path& allPath : potentialPaths)
		{
			allPath.pop_front();
			if (allPath.empty())
				pathsToRemove.push_back(allPath);
		}

		for (MAPF::Path& pathToRemove : pathsToRemove)
		{
			auto it = std::find(potentialPaths.begin(), potentialPaths.end(), pathToRemove);
			if (it != potentialPaths.end())
				potentialPaths.erase(it);
		}

		for (MAPF::Path& allPath : potentialPaths)
			assert(!allPath.empty());
	}


	if (chosenPath.empty()) // we have reached our goal
		goal = nullptr;
}

void Agent::SetPath(MAPF::Path& inPath)
{
	chosenPath = inPath;
}

void Agent::Update(float dt)
{
	renderPos.x += (x - renderPos.x) * dt * 10.f;
	renderPos.y += (y - renderPos.y) * dt * 10.f;
}

void Agent::DrawPaths(sf::RenderWindow& window)
{
	float pathSep = .4 / potentialPaths.size();

	for (int pathIndex = 0; pathIndex < potentialPaths.size(); pathIndex++)
	{
		MAPF::Path& path = potentialPaths[pathIndex];
		if (path.empty()) continue;

		sf::VertexArray points(sf::Lines, path.size());

		for (int tileIndex = 0; tileIndex < path.size(); tileIndex++)
		{
			Tile* tile = path[tileIndex];
			sf::Vector2f pos(tile->x, tile->y);
			points[tileIndex] = sf::Vertex(sf::Vector2f(tile->x + pathSep * pathIndex - 0.2f, tile->y + pathSep * pathIndex - 0.2f), color);
		}

		window.draw(&points[0], path.size(), sf::Lines);
 	}
}

void Agent::DrawLineToGoal(sf::RenderWindow& window)
{
	if (!goal) return;
	sf::Color lineColor = color;
	lineColor.a = 100;
	sf::Vertex points[2];
	points[0] = sf::Vertex(sf::Vector2f(x, y), lineColor);
	points[1] = sf::Vertex(sf::Vector2f(goal->x, goal->y), lineColor);

	window.draw(points, 2, sf::Lines);
}

void Agent::DrawGoal(sf::RenderWindow& window)
{
	if (!goal) return;

	sf::Vector2f rectSize(0.3f, 0.3f);
	sf::RectangleShape rect(rectSize);
	rect.setOrigin(rectSize * 0.5f);
	rect.setPosition(sf::Vector2f(goal->x, goal->y));
	rect.setFillColor(color);
	window.draw(rect);
}

std::ostream& operator<<(std::ostream& os, Agent& agent)
{
	os << "Agent " << agent.GetAgentId() << "[" << agent.x << "," << agent.y << " | Goal " << *agent.goal << "]";
	return os;
}
