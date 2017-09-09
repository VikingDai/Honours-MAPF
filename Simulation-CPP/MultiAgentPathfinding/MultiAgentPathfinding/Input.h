#pragma once

//class Camera;
class Simulation;

class Input
{
public:
	void Update(float deltaTime, Simulation* simulation);
	void StepSimulation(Simulation* simulation);
};


