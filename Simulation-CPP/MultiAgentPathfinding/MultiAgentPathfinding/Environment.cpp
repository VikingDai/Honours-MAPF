#include "Environment.h"
#include <iostream>
#include "glm/vec3.hpp"
#include "TemporalAStar.h"
#include "Agent.h"

#include <SFML/Graphics.hpp>
#include "Globals.h"

#define DEBUG_VERBOSE 0
#define DEBUG_STATS 0

Environment::Environment()
{
	std::cout << "Created environment" << std::endl;
}

void Environment::Reset()
{
	Agent::ResetAgentCounter();

	for (Agent* agent : agents)
		delete agent;
	agents.clear();

	for (EObject* object : objects)
		delete object;
	objects.clear();
}

Environment::~Environment()
{
	Reset();
}

void Environment::Step()
{
	for (Agent* agent : agents)
		agent->Step();
}

void Environment::Render(sf::RenderWindow& window)
{
	/** draw grid map texture */
	sf::Sprite sprite(gridMapRenderTexture.getTexture());
	sprite.setScale(sf::Vector2f(0.1f, 0.1f) * Globals::renderSize);
	sprite.setPosition(sf::Vector2f(-0.5f, -0.5f) * Globals::renderSize);
	window.draw(sprite);
}

bool Environment::GenerateGridMapTexture()
{
	Timer timerGridMap;
	timerGridMap.Begin();

	if (!gridMapRenderTexture.create(
		gridMap.GetWidth()  * 10.f,
		gridMap.GetHeight() * 10.f))
	{
		std::cout << "Failed to create grid map texture" << std::endl;
		return false;
	}

	gridMapRenderTexture.clear(sf::Color(50, 50, 50));

	for (Tile* tile : gridMap.tiles)
	{
		DrawGridMapTile(tile->x, tile->y, tile->GetColor());
		//sf::Vector2f& rectSize = sf::Vector2f(10.f, 10.f);
		//sf::RectangleShape rect(rectSize);
		//rect.setOrigin(rectSize * 0.5f);

		//sf::Vector2f& rectPos =
		//	sf::Vector2f(tile->x, tile->y) * 10.f + // scaled pos 
		//	rectSize * 0.5f; // offset

		//rect.setPosition(rectPos);
		//rect.setOutlineColor(sf::Color::Black);
		//rect.setOutlineThickness(rectSize.x * -0.1f);
		//sf::Color color = tile->isWalkable ? sf::Color::White : sf::Color::Black;
		//rect.setFillColor(color);

		//gridMapRenderTexture.draw(rect);
	}

	gridMapRenderTexture.display();

	timerGridMap.End();
#if DEBUG_STATS
	timerGridMap.PrintTimeElapsed("Rendering grid map onto texture");
#endif

	return true;
}

void Environment::DrawGridMapTile(int x, int y, sf::Color color)
{
	sf::Vector2f& rectSize = sf::Vector2f(10.f, 10.f);
	sf::RectangleShape rect(rectSize);
	rect.setOrigin(rectSize * 0.5f);

	sf::Vector2f& rectPos = sf::Vector2f(x, y) * 10.f + // scaled pos 
		rectSize * 0.5f; // offset

	rect.setPosition(rectPos);
	rect.setOutlineColor(sf::Color::Black);
	rect.setOutlineThickness(rectSize.x * -0.1f);
	rect.setFillColor(color);

	gridMapRenderTexture.draw(rect);
}

void Environment::DrawGridMapTile(Tile* tile)
{
	if (tile->isDirty)
	{
		tile->isDirty = false;
		DrawGridMapTile(tile->x, tile->y, tile->GetColor());
	}
}

void Environment::LoadMap(std::string mapName)
{
	gridMap.LoadMap(mapName);
	freeStartTiles = gridMap.walkableTiles;
	freeGoalTiles = gridMap.walkableTiles;
	freeTiles = gridMap.walkableTiles;
}

void Environment::AddAgent(Agent* agent)
{
	Tile* start = gridMap.GetTileAt(agent->x, agent->y);
	freeStartTiles.erase(std::remove(freeStartTiles.begin(), freeStartTiles.end(), start), freeStartTiles.end());
	freeTiles.erase(std::remove(freeTiles.begin(), freeTiles.end(), start), freeTiles.end());

	if (Tile* goal = agent->goal)
	{
		freeGoalTiles.erase(std::remove(freeGoalTiles.begin(), freeGoalTiles.end(), goal), freeGoalTiles.end());
		freeTiles.erase(std::remove(freeTiles.begin(), freeTiles.end(), goal), freeTiles.end());
	}

	agents.push_back(agent);
}

void Environment::GenerateRandomAgents(int numToGenerate)
{
	/** load random agents */
	for (int i = 0; i < numToGenerate; i++)
	{
		Tile* start = freeStartTiles[rand() % (freeStartTiles.size() - 1)]; // pick a random free tile

		Tile* goal = freeGoalTiles[rand() % (freeGoalTiles.size() - 1)];

		/** make sure the agent's goal is not equal to their start tile! */
		while (goal == start)
			goal = freeGoalTiles[rand() % (freeGoalTiles.size() - 1)];

		assert(start);
		assert(goal);

		Agent* agent = new Agent(&gridMap, start, goal);
		AddAgent(agent);

#if DEBUG_VERBOSE
		std::cout << "Randomly spawned " << *agent << " at " << agent->x << ", " << agent->y << std::endl;
#endif
	}
}

void Environment::FillWithObstacles(float percentage)
{
	int numTiles = gridMap.GetNumTiles();
	int tilesToSpawn = floor(numTiles * percentage);

	for (int i = 0; i < tilesToSpawn; i++)
	{
		Tile* tile = freeTiles[rand() % (freeTiles.size() - 1)];
		gridMap.SetObstacle(tile);
	}
}

void Environment::GetFreeTiles(std::vector<Tile*>& freeStartTiles, std::vector<Tile*>& freeGoalTiles)
{
	/** These two vectors ensure that no two agents share the same start tile
	AND no two agents share the same goal tile */
	freeStartTiles = gridMap.walkableTiles;
	freeGoalTiles = gridMap.walkableTiles;

	/** remove start and goal locations from free tiles */
	for (Agent* agent : agents)
	{
		Tile* start = gridMap.GetTileAt(agent->x, agent->y);
		freeStartTiles.erase(std::remove(freeStartTiles.begin(), freeStartTiles.end(), start), freeStartTiles.end());

		if (Tile* goal = agent->goal)
			freeGoalTiles.erase(std::remove(freeGoalTiles.begin(), freeGoalTiles.end(), goal), freeGoalTiles.end());
	}
}
