#pragma once
#include "EObject.h"
#include <vector>
#include <deque>
#include <iostream>
#include "TemporalAStar.h"
#include "Graphics.h"
#include "TemporalBFS.h"

class Tile;
class GridMap;

class Agent : public EObject
{
public:
	TemporalBFS* bfs;

private:
	TemporalAStar* search;
	int agentId;


	TemporalAStar::Path currentPath;
public:
	std::vector<TemporalAStar::Path> allPaths;
	void setPath(TemporalAStar::Path& path);
	TemporalAStar::Path& getPath() { return currentPath; }

public:
	Tile* goal;

	vec3 color;
	Agent(GridMap* gridMap, Tile* startTile, Tile* goalTile = nullptr);
	virtual void step();
	

	std::map<Tile*, int> customWeights;

	int getAgentId() { return agentId; }

	vec3 renderPos;
	void update(float dt);

	void drawPaths(Graphics* graphics);

	friend std::ostream& operator<<(std::ostream& os, Agent& agent);
};

