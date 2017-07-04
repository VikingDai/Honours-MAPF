#pragma once
#include "EObject.h"

class Agent : public EObject
{
private:


public:
	virtual void Step();
	virtual void Render() override;
};

