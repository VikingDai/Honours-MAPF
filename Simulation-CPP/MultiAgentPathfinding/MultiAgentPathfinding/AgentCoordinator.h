#pragma once
#include <vector>
#include <map>
#include <queue>
#include <deque>
#include <set>
#include <unordered_set>
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
	AStar aStar;
	GridMap* gridMap;
	PathAssigner pathAssigner;

	std::vector<MAPF::AgentPathRef*> usedPathRefs;

public:
	AgentCoordinator(GridMap* map);
	~AgentCoordinator();
	
	Timer timerCollisionDetection;
	Timer timerPathAssignment;
	Timer timerCoordinator;
	Timer timerPathGeneration;

private:
	int iteration;
	bool isRunning;
	
private:
	void PrintPath(Agent* agent, MAPF::Path& path);

private:
	using TileToPathMap = std::map<Tile*, std::vector<MAPF::AgentPathRef*>>;
	
	/** (tile => time => num collisions) */
	using TileCollision = std::vector<std::pair<Tile*, int>>;

	/** This path is in collision with other paths at time*/
	using CollisionAtTime = std::map<int, std::vector<MAPF::AgentPathRef*>>;

private:
	std::map<Agent*, CollisionPenalties> agentPenalties;
	std::map<Agent*, TemporalAStar::TileCosts> agentCollisionPenalties;
	std::map<Agent*, std::map<Agent*, int>> agentCollisionCount;

	std::set<Agent*> failedAgents;
	std::map<Agent*, TileCollision> agentCollisionMap;
	TemporalAStar::TileCosts collisionCosts;
	std::deque<TileToPathMap> tileToPathMapAtTimestep;
	std::map<Tile*, std::map<int, std::vector<MAPF::AgentPathRef*>>> collisionTable;

	std::map<Agent*, std::vector<std::pair<int, Tile*>>> agentCrossCollisions;

	std::vector<std::pair<Tile*, MAPF::AgentPathRef*>> bottomLayer;

private:
	bool Init(std::vector<Agent*>& agents);

	MAPF::PathCollisions crossCollisionSet;
public:
	void Reset();

	int GetLongestPathLength(std::vector<Agent*>& agents);
	Tile* GetTileAtTimestep(MAPF::Path& path, int timestep);

	void UpdateAgents2(std::vector<Agent*>& agents);
	std::vector<MAPF::PathCollision> CheckForCollisions(std::vector<Agent*>& agents);

	void CreateActionPenalties(CollisionPenalties& penalties, MAPF::AgentPathRef* pathRef);

	void GeneratePath2(MAPF::PathCollision& collision);
};