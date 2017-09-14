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

	Tile* hoveredTile;

	float zoom;

	Input(Simulation* simulation, sf::RenderWindow* window, sf::View* view) 
		: simulation(simulation), window(window), view(view),
		zoom(1.f), hoveredTile(nullptr)
		
	{
	}

	void ProcessInput(sf::Event& event);
	void OnKeyPressed(sf::Event& event);
	void OnMousePressed(sf::Event& event);
	void OnMouseReleased(sf::Event& event);

	void Update(float deltaTime, Simulation* simulation);
	void StepSimulation(Simulation* simulation);
};


