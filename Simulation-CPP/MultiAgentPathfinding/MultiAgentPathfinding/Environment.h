#pragma once
#include <vector>
#include "EObject.h"
#include "GridMap.h"
#include "Agent.h"
#include <SFML/Graphics/RenderWindow.hpp>

class Graphics;

class Environment
{
public:
	std::vector<Agent*> agents;
	std::vector<EObject*> objects;
	GridMap gridMap;

public:
	Environment();
	~Environment();

	void Reset();
	void Step();
	void Render(Graphics* graphics);
	void Render2(sf::RenderWindow& window);
	
};

