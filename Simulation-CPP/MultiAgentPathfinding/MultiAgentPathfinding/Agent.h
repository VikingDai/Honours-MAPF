#pragma once
#include "EObject.h"

class Agent : public EObject
{
private:


public:
	Agent(int x, int y) : EObject(x, y) {};
	virtual void Step();
};

