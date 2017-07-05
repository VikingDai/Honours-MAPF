#include "Simulation.h"
#include <iostream>
#include "Graphics.h"


Simulation::Simulation()
{
	timestep = 0;
	aStar = new AStar(&environment.gridMap);
}


Simulation::~Simulation()
{
}

void Simulation::Step()
{
	std::vector<Tile*> path = aStar->findPath(environment.gridMap.getTileAt(1, 1), environment.gridMap.getTileAt(15, 18));

	for (Tile* tile : path)
	{
		std::cout << tile->x << "," << tile->y << std::endl;
	}

	std::cout << "Updating timestep: " << timestep << std::endl;

	timestep += 1;
}

void Simulation::Render(Graphics* graphics)
{
	environment.Render(graphics);
}
