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

	/** These two vectors ensure that no two agents share the same start tile or the same goal tile */
	std::vector<Tile*> freeStartTiles;
	std::vector<Tile*> freeGoalTiles;
	std::vector<Tile*> freeTiles;

public:
	Environment();
	~Environment();

	void Reset();
	void Step();
	void Render(sf::RenderWindow& window);

	bool GenerateGridMapTexture();

public:	
	void LoadMap(std::string mapName);
	void AddAgent(Agent* agent);

	void GenerateRandomAgents(int numToGenerate);
	void FillWithObstacles(float percentage);

	void GetFreeTiles(std::vector<Tile*>& freeStartTiles, std::vector<Tile*>& freeGoalTiles);
};

