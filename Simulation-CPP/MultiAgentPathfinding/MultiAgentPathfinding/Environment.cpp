#include "Environment.h"
#include <iostream>
#include "glm/vec3.hpp"
#include "TemporalAStar.h"

#include <SFML/Graphics.hpp>

Environment::Environment()
{
	std::cout << "Loaded environment" << std::endl;
	//gridMap.loadMap("../maps/straightMap.map");
	//gridMap.loadMap("../maps/squareMap.map");
	//gridMap.loadMap("../maps/warehouse.map");
	//gridMap.loadMap("../maps/maze512-1-8.map");

	//gridMap.loadMap("../maps/waitTest.map");

	//for (int i = 0; i < 1; i++)
	//{
	//	Tile* randomTile = gridMap.walkableTiles[rand() % gridMap.walkableTiles.size()];
	//	agents.push_back(new Agent(randomTile));
	//}
}

void Environment::Reset()
{
	agents.clear();
	objects.clear();
}

Environment::~Environment()
{
}

void Environment::Step()
{
	for (Agent* agent : agents)
		agent->Step();
}

void Environment::Render(sf::RenderWindow& window)
{
	// draw grid map texture
	sf::Sprite sprite(gridMapRenderTexture.getTexture());
	sprite.setScale(sf::Vector2f(0.1f, 0.1f));
	sprite.setPosition(sf::Vector2f(-0.5f, -0.5f));
	window.draw(sprite);

	// draw the agents as circles
	for (Agent* agent : agents)
	{
		agent->Update(0.016f);

		float circleRadius = -0.4f;
		sf::CircleShape circ(circleRadius);
		circ.setOrigin(sf::Vector2f(circleRadius, circleRadius));
		circ.setPosition(sf::Vector2f(agent->renderPos.x, agent->renderPos.y));
		circ.setFillColor(agent->color);
		window.draw(circ);
	}
}

bool Environment::GenerateGridMapTexture()
{
	Timer timerGridMap;
	timerGridMap.Begin();

	if (!gridMapRenderTexture.create(gridMap.getWidth() * 10.f, gridMap.getHeight() * 10.f))
		return false;

	gridMapRenderTexture.clear(sf::Color::Black);

	for (Tile* tile : gridMap.tiles)
	{
		sf::Color color = tile->isWalkable ? sf::Color::White : sf::Color::Black;

		sf::Vector2f rectSize(10.f, 10.f);
		sf::RectangleShape rect(rectSize);
		rect.setOrigin(rectSize * 0.5f);
		rect.setPosition(sf::Vector2f(tile->x, tile->y) * 10.f + rectSize * 0.5f);
		rect.setOutlineColor(sf::Color::Black);
		rect.setOutlineThickness(-1.f);
		rect.setFillColor(color);

		gridMapRenderTexture.draw(rect);
	}

	gridMapRenderTexture.display();

	timerGridMap.End();
	timerGridMap.PrintTimeElapsed("Rendering grid map onto texture");

	return true;
}
