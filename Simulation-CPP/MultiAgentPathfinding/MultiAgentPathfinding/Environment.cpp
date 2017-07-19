#include "Environment.h"
#include <iostream>
#include "Graphics.h"
#include "glm/vec3.hpp"
#include "AStar.h"

Environment::Environment()
{
	std::cout << "Loaded environment" << std::endl;
	gridMap.loadMap("../maps/straightMap.map");
	//gridMap.loadMap("../maps/squareMap.map");
	//gridMap.loadMap("../maps/warehouse.map");
	//gridMap.loadMap("../maps/maze512-1-8.map");

	for (int i = 0; i < 2; i++)
	{
		Tile* randomTile = gridMap.walkableTiles[rand() % gridMap.walkableTiles.size()];
		agents.push_back(new Agent(randomTile->x, randomTile->y));
	}
}

Environment::~Environment()
{
}

void Environment::Step()
{
	for (Agent* agent : agents)
		agent->step();
}

void Environment::Render(Graphics* graphics)
{
	// draw the grid map tiles as squares
	graphics->ShapeBatchBegin(SHAPE_SQUARE);
	for (Tile* tile : gridMap.tiles)
	{
		vec3 color = tile->isWalkable ? vec3(1) : vec3(0);

		if (tile->color != vec3(0))
			color = tile->color;

		graphics->DrawBatch(vec3(tile->x, tile->y, 0), color, vec3(0.9f));
	}
	graphics->ShapeBatchEnd();

	// draw the agents as circles
	graphics->ShapeBatchBegin(SHAPE_CIRCLE);
	for (Agent* agent : agents)
	{
		agent->update(0.016f);
		graphics->DrawBatch(agent->renderPos, agent->color, vec3(0.8f));
		//graphics->DrawBatch(vec3(agent->x, agent->y, 0), agent->color, vec3(0.8f));
	}
	graphics->ShapeBatchEnd();

	//// draw agent's paths
	//for (Agent* agent : agents)
	//{
	//	std::vector<ivec3> points;
	//	for (Tile* tile : agent->getPath())
	//		points.emplace_back(vec3(tile->x, tile->y, 0));

	//	graphics->DrawLine(points, agent->color);//vec3(0, 0, 0));
	//}
}
