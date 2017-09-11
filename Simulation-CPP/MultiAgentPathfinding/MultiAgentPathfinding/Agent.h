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
	TemporalBFS* bfs;
	TemporalAStar* temporalAStar;
	AStar* aStar;

private:
	int agentId;

	MAPF::Path chosenPath;
public:
	std::vector<MAPF::Path> potentialPaths;

	void SetPath(MAPF::Path& path);
	MAPF::Path& GetPath() { return chosenPath; }

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

	friend std::ostream& operator<<(std::ostream& os, Agent& agent);
};

struct AgentPathRef
{
	Agent* agent;
	int pathIndex;

	AgentPathRef(Agent* agent, int pathIndex) : agent(agent), pathIndex(pathIndex) {}

	MAPF::Path& GetPath() { return agent->potentialPaths[pathIndex]; }

	friend std::ostream& operator<<(std::ostream& os, AgentPathRef& pathRef)
	{
		os << "Path " << pathRef.pathIndex << ": Agent(" << pathRef.agent->GetAgentId() << ")";
		return os;
	}
};