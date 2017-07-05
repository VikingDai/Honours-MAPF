#pragma once
#include "Mesh.h"


class Tile
{

public:
	Mesh* mesh;
	int x, y;
	Tile(int x, int y);
};

