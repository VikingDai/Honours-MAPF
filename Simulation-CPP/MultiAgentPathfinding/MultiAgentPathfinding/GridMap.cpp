#include "GridMap.h"
#include <fstream>
#include <string>
#include <iostream>


GridMap::GridMap()
{
	width = height = 0;
}


GridMap::~GridMap()
{
}

void GridMap::loadMap(std::string filename)
{
	walkableTiles.clear();
	tiles.clear();
	tileGrid.clear();

	std::fstream infile(filename);

	assert(infile);

	// line #1: type mapType
	// line #2: height h
	// line #3: width w
	// line #4: map
	// line #5 to end: map data

	std::string dummyText;
	infile >> dummyText >> mapType;
	infile >> dummyText >> height;
	infile >> dummyText >> width;
	infile >> dummyText;

	numTiles = width * height;

	printf("Loaded map: %s | Map Type: %s | Width: %d | Height %d\n",
		filename.c_str(), mapType.c_str(), width, height);

	getline(infile, mapType);

	tileGrid.resize(width * height);

	//int y = 0;
	int index = 0;
	for (std::string line; getline(infile, line);)
	{
		for (char& c : line)
		{
			int x = getTileX(index);
			int y = getTileY(index);

			bool isWalkable = c == ' ' || c == '.';

			Tile* tile = new Tile(x, y, isWalkable);
			tileGrid[index] = tile;
			tiles.push_back(tile);
			if (isWalkable) walkableTiles.push_back(tile);

			index += 1;
		}
	}
}

int GridMap::getTileIndex(int x, int y) const
{
	return x + y * width;
}

int GridMap::getTileX(int index) const
{
	return index % width;
}

int GridMap::getTileY(int index) const
{
	return index == 0 ? 0 : static_cast<int>(floor(static_cast<float>(index) / static_cast<float>(width)));
}

Tile* GridMap::getTileAt(int index) const
{
	bool inBounds = index >= 0 && index < width * height;
	return inBounds ? tileGrid[index] : nullptr;
}

Tile* GridMap::getTileAt(int x, int y) const
{
	bool inBounds = x >= 0 && x < width;
	return inBounds ? getTileAt(getTileIndex(x, y)) : nullptr;
}

Tile* GridMap::getTileRelativeTo(const Tile* tile, int x, int y)
{
	return getTileAt(tile->x + x, tile->y + y);
}

bool GridMap::isWalkable(const int x, const int y) const
{
	Tile* tile = getTileAt(x, y);
	return tile && tile->isWalkable;
}

Tile* GridMap::randomWalkableTile()
{
	return walkableTiles[rand() % walkableTiles.size()];
}
