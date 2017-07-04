#include "Simulation.h"
#include <iostream>



Simulation::Simulation()
{
	timestep = 0;
}


Simulation::~Simulation()
{
}

void Simulation::Step()
{
	std::cout << "Updating timestep: " << timestep << std::endl;

	timestep += 1;
}

void Simulation::Render()
{
	
}
