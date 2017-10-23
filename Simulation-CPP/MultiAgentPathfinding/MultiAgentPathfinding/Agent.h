#pragma once
#include "EObject.h"
#include <vector>
#include <deque>
#include <iostream>
#include "TemporalAStar.h"
#include "TemporalBFS.h"
#include "AStar.h"
#include "MAPF.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>

class Tile;
class GridMap;

class Agent : public EObject
{
public:
	Agent(GridMap* gridMap, Tile* startTile, Tile* goalTile = nullptr);

private:
	static int agentCounter;
public:
	static void ResetAgentCounter() { agentCounter = 0; }

public:
	TemporalBFS bfs;
	TemporalAStar temporalAStar;

private:
	int agentId;

	MAPF::AgentPathRef* pathRef;
public:
	std::vector<MAPF::Path> pathBank;

	MAPF::Path& GetAssignedPath() 
	{ 
		if (pathRef)
			return pathRef->GetPath();
		else
			return MAPF::Path();
	}

	void SetPath(MAPF::AgentPathRef* pathRef);
	MAPF::AgentPathRef* GetPathRef() { return pathRef; }

public:
	Tile* goal;

	sf::Color color;
	sf::Text textAgentId;
	
	virtual void Step();
	

	std::map<Tile*, int> customWeights;

	int GetAgentId() { return agentId; }

	sf::Vector2f renderPos;
	void Update(float dt);

	void DrawPath(sf::RenderWindow& window);
	void DrawPotentialPaths(sf::RenderWindow& window);
	void DrawLineToGoal(sf::RenderWindow& window);
	void DrawGoal(sf::RenderWindow& window);
	void DrawAgent(sf::RenderWindow& window);

	MAPF::AgentPathRef* GeneratePath(GridMap* gridMap, CollisionPenalties& penalties, std::vector<MAPF::AgentPathRef*>& usedPathRefs = std::vector<MAPF::AgentPathRef*>());

	friend std::ostream& operator<<(std::ostream& os, Agent& agent);
};