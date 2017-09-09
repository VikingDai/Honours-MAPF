#include "Environment.h"
#include <iostream>
#include "glm/vec3.hpp"
#include "TemporalAStar.h"
#include "Agent.h"

#include <SFML/Graphics.hpp>
#include "Globals.h"

Environment::Environment()
{
	std::cout << "Created environment" << std::endl;
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
		gridMap.getWidth()  * 10.f,
		gridMap.getHeight() * 10.f))
	{
		std::cout << "Failed to create grid map texture" << std::endl;
		return false;
	}

	gridMapRenderTexture.clear(sf::Color(50, 50, 50));

	for (Tile* tile : gridMap.tiles)
	{
		sf::Vector2f& rectSize = sf::Vector2f(10.f, 10.f);
		sf::RectangleShape rect(rectSize);
		rect.setOrigin(rectSize * 0.5f);

		sf::Vector2f& rectPos = 
			sf::Vector2f(tile->x, tile->y) * 10.f + // scaled pos 
			rectSize * 0.5f; // offset

		rect.setPosition(rectPos);
		rect.setOutlineColor(sf::Color::Black);
		rect.setOutlineThickness(rectSize.x * -0.1f);
		sf::Color color = tile->isWalkable ? sf::Color::White : sf::Color::Black;
		rect.setFillColor(color);
		
		gridMapRenderTexture.draw(rect);
	}

	gridMapRenderTexture.display();

	timerGridMap.End();
	timerGridMap.PrintTimeElapsed("Rendering grid map onto texture");

	return true;
}
