#pragma once
#include <vector>
#include "EObject.h"
#include "GridMap.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

class Graphics;
class Agent;

class Environment
{
public:
	std::vector<Agent*> agents;
	std::vector<EObject*> objects;
	GridMap gridMap;
	sf::RenderTexture gridMapRenderTexture;

public:
	Environment();
	~Environment();

	void Reset();
	void Step();
	void Render(sf::RenderWindow& window);

	bool GenerateGridMapTexture();
};

