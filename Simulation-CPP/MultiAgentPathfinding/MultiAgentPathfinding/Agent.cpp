#include "Agent.h"
#include "TemporalAStar.h"
#include "Tile.h"
#include "GridMap.h"
#include "MathUtils.h"

#include <SFML/Graphics.hpp>
#include "Globals.h"
#include "Graphics.h"
#include "MAPF.h"

int Agent::agentCounter = 0;

Agent::Agent(GridMap* gridMap, Tile* startTile, Tile* goalTile) 
	: EObject(startTile->x, startTile->y), 
	bfs(gridMap), temporalAStar(gridMap),
	goal(goalTile), pathRef(nullptr)
{
	assert(startTile);

	color = sf::Color(0, MathUtils::RandomFloat() * 255, MathUtils::RandomFloat() * 255);

	agentId = agentCounter;
	agentCounter += 1;

	textAgentId.setCharacterSize(24);
	textAgentId.setString(std::to_string(agentId));
	textAgentId.setFont(Globals::FONT_DROID_SANS);
	textAgentId.setColor(sf::Color(255 - color.r, 255 - color.b, 255 - color.g));

	renderPos = sf::Vector2f(startTile->x, startTile->y);
}

void Agent::Step()
{
	if (GetPathRef() && !GetAssignedPath().empty())
	{
		// move along current path p to us by the agent coordinator
		Tile* nextTile = GetAssignedPath().front();
		GetAssignedPath().pop_front();

		x = nextTile->x;
		y = nextTile->y;

		//std::vector<MAPF::Path> pathsToRemove;
		//for (MAPF::Path& allPath : potentialPaths)
		//{
		//	if (allPath.empty())
		//		pathsToRemove.push_back(allPath);
		//	else
		//		allPath.pop_front();
		//}

		//for (MAPF::Path& pathToRemove : pathsToRemove)
		//{
		//	auto it = std::find(potentialPaths.begin(), potentialPaths.end(), pathToRemove);
		//	if (it != potentialPaths.end())
		//		potentialPaths.erase(it);
		//}
	}


	if (GetAssignedPath().empty()) // we have reached our goal
		goal = nullptr;
}

//void Agent::SetPath(MAPF::Path& inPath)
//{
//	chosenPath = inPath;
//}

void Agent::SetPath(MAPF::AgentPathRef* pathRef)
{
	this->pathRef = pathRef;
}

void Agent::Update(float dt)
{
	renderPos.x += (x - renderPos.x) * dt * 10.f;
	renderPos.y += (y - renderPos.y) * dt * 10.f;
}

void Agent::DrawPath(sf::RenderWindow& window)
{
	if (!pathRef)
		return;

	MAPF::Path& path = GetPathRef()->GetPath();

	if (path.empty())
		return;

	sf::VertexArray points(sf::LineStrip, path.size());
	for (int tileIndex = 0; tileIndex < path.size(); tileIndex++)
	{
		Tile* tile = path[tileIndex];
		points[tileIndex] = sf::Vertex(sf::Vector2f(tile->x, tile->y) * Globals::renderSize, color);
	}

	window.draw(&points[0], path.size(), sf::LineStrip);
}

void Agent::DrawPotentialPaths(sf::RenderWindow& window)
{
	float pathSep = .4 / pathBank.size();

	for (int pathIndex = 0; pathIndex < pathBank.size(); pathIndex++)
	{
		MAPF::Path& path = pathBank[pathIndex];
		if (path.empty()) continue;

		sf::VertexArray points(sf::LineStrip, path.size());

		for (int tileIndex = 0; tileIndex < path.size(); tileIndex++)
		{
			Tile* tile = path[tileIndex];
			sf::Vector2f pos(tile->x, tile->y);
			points[tileIndex] = sf::Vertex(sf::Vector2f(tile->x + pathSep * pathIndex - 0.2f, tile->y + pathSep * pathIndex - 0.2f) * Globals::renderSize, color);
		}

		window.draw(&points[0], path.size(), sf::LineStrip);
 	}
}

void Agent::DrawLineToGoal(sf::RenderWindow& window)
{
	if (!goal) return;
	sf::Color lineColor = color;
	lineColor.a = 100;
	sf::Vertex points[2];
	points[0] = sf::Vertex(sf::Vector2f(x, y) * Globals::renderSize, lineColor);
	points[1] = sf::Vertex(sf::Vector2f(goal->x, goal->y) * Globals::renderSize, lineColor);

	window.draw(points, 2, sf::Lines);
}

void Agent::DrawGoal(sf::RenderWindow& window)
{
	if (!goal) return;

	sf::Vector2f& rectSize = sf::Vector2f(0.3f, 0.3f) * Globals::renderSize;
	sf::RectangleShape rect(rectSize);
	rect.setOrigin(rectSize * 0.5f);
	rect.setPosition(sf::Vector2f(goal->x, goal->y) * Globals::renderSize);
	rect.setFillColor(color);
	window.draw(rect);
}

void Agent::DrawAgent(sf::RenderWindow& window)
{
	float circleRadius = 0.4f * Globals::renderSize;
	sf::CircleShape circ(circleRadius);
	circ.setOrigin(sf::Vector2f(circleRadius, circleRadius));
	circ.setPosition(renderPos * Globals::renderSize);
	circ.setFillColor(color);
	window.draw(circ);

	textAgentId.setPosition(renderPos * Globals::renderSize);
	textAgentId.setOrigin(sf::Vector2f(textAgentId.getCharacterSize(), textAgentId.getCharacterSize()) * 0.5f);
	window.draw(textAgentId);
}

void Agent::GeneratePath(MAPF::Path& outPath, GridMap* gridMap)
{
	outPath = temporalAStar.FindPath(gridMap->GetTileAt(x, y), goal, penalties);
}

MAPF::AgentPathRef* Agent::AddToPathBank(MAPF::Path& path, std::vector<MAPF::AgentPathRef*>& usedPathRefs)
{
	// check if the path is already in the path bank
	if (std::find(pathBank.begin(), pathBank.end(), path) != pathBank.end())
		return nullptr;
	
	// if not then add it
	pathBank.push_back(path);
	//pathBank.emplace_back(path);
	return new MAPF::AgentPathRef(this, pathBank.size() - 1);
	//return MAPF::AgentPathRef::Make(this, pathBank.size() - 1, usedPathRefs);
}

std::ostream& operator<<(std::ostream& os, Agent& agent)
{
	os << "Agent " << agent.GetAgentId(); //<< "[" << agent.x << "," << agent.y << " | Goal " << *agent.goal << "]";
	return os;
}

