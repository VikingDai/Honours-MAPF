#include "Environment.h"
#include <iostream>



Environment::Environment()
{
	std::cout << "Loaded environment" << std::endl;
	gridMap.loadMap("../maps/tinyMap.map");
}


Environment::~Environment()
{
}
