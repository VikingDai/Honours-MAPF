#pragma once
#include <vector>
#include <map>
#include <queue>
#include <deque>
#include <set>
#include "Agent.h";
#include "TemporalAStar.h"

#include "Timer.h"
#include "PathAssigner.h"

#include "MAPF.h"

#include <SFML/Graphics/RenderWindow.hpp>

class GridMap;
class Tile;

class AgentCoordinator
{
private:
	TemporalAStar* aStar;
	GridMap* gridMap;
	PathAssigner* pathAssigner;

public:
	AgentCoordinator(GridMap* map);
	~AgentCoordinator();
	
private:
	int iteration;
	bool isRunning;
	Timer coordinatorTimer;
	Timer generatePathTimer;

private:
	void PrintPath(Agent* agent, MAPF::Path& path);

private:
	using TileToPathMap = std::map<Tile*, std::vector<AgentPathRef*>>;
	
	/** (tile => time => num collisions) */
	using TileCollision = std::vector<std::pair<Tile*, int>>;

	/** This path is in collision with other paths at time*/
	using CollisionAtTime = std::map<int, std::vector<AgentPathRef*>>;

private:
	std::map<Agent*, TemporalAStar::TileCosts> agentCollisionCosts;
	std::map<Agent*, std::map<Agent*, int>> agentCollisionCount;

	std::set<Agent*> agentsRequiringPath;
	std::map<Agent*, TileCollision> agentCollisionMap;
	TemporalAStar::TileCosts collisionCosts;
	std::deque<TileToPathMap> tileToPathMapAtTimestep;
	std::map<Tile*, std::map<int, std::vector<AgentPathRef*>>> collisionTable;

	std::vector<std::pair<Tile*, AgentPathRef*>> bottomLayer;

private:
	bool Init(std::vector<Agent*>& agents);

	/** Generates a new path for an agent */
	void GeneratePath(
		Agent* agent, 
		bool firstRun);

	using CollisionSet = std::vector<std::set<AgentPathRef*>>;
	/** Groups together paths which are in collision with one another */

	std::set<std::pair<Tile*, int>> tilesInCollision;
	CollisionSet DetectTileCollisions();
	CollisionSet crossCollisionSet;

	/** */
	//void BuildCollisionTable(std::vector<Agent*>& agents);

	/** Updates the collision table and stores any path collisions */
	CollisionAtTime UpdateCollisions(AgentPathRef* agentPathRef);

public:
	void Reset();

	/** Gives each agent a conflict-free path */
	void UpdateAgents(std::vector<Agent*>& agents);

	/** Generate paths and attempt to resolve conflicts for one iteration */
	bool Step(std::vector<Agent*>& agents);

	void RenderCollisionCosts(sf::RenderWindow& window);
};

