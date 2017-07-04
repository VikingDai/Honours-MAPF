#pragma once
#include "Simulation.h"
class Input
{
public:
	void Update(float deltaTime, Simulation* simulation);
	void StepSimulation(Simulation* simulation);
};


