#pragma once

#include <SFML/Graphics.hpp>
#include "Tile.h"

class Simulation;

class Input
{
public:
	Simulation* simulation;
	sf::View* view;
	sf::RenderWindow* window;

	Tile* startTile;
	Tile* goalTile;
	Tile* hoveredTile;

	float zoom;

	Input(Simulation* simulation, sf::RenderWindow* window, sf::View* view) 
		: simulation(simulation), window(window), view(view),
		zoom(1.f), 
		hoveredTile(nullptr), startTile(nullptr), goalTile(nullptr)
	{
	}

	sf::Vector2f GetMouseWorld();
	Tile* GetTileUnderCursor();

	void ProcessInput(sf::Event& event);
	void OnKeyPressed(sf::Event& event);
	void OnMousePressed(sf::Event& event);
	void OnMouseReleased(sf::Event& event);

	void Update(float deltaTime, Simulation* simulation);
	void StepSimulation(Simulation* simulation);

	void Reset();
};


