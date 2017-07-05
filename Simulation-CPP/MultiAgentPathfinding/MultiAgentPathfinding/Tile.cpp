#include "Tile.h"
#include <iostream>
#include <glm/vec3.hpp>

Tile::Tile(int inX, int inY)
{
	x = inX; y = inY;
	mesh = new Mesh(glm::vec3(x, y, 0));
	std::cout << "x:" << x << "y:" << y  << std::endl;
}