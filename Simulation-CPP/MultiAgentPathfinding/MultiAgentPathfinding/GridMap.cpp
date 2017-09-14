#include "GridMap.h"
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>

#define DEBUG_VERBOSE 0

GridMap::GridMap()
{
	width = height = 0;
}


GridMap::~GridMap()
{
	for (Tile* tile : tiles)
		delete tile;
}

void GridMap::LoadMap(std::string filename)
{
	walkableTiles.clear();
	tileGrid.clear();

	for (Tile* tile : tiles)
		delete tile;
	tiles.clear();
	


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

#if DEBUG_VERBOSE
	printf("Loaded map: %s | Map Type: %s | Width: %d | Height %d\n",
		filename.c_str(), mapType.c_str(), width, height);
#endif

	getline(infile, mapType);

	tileGrid.resize(width * height);

	//int y = 0;
	int index = 0;
	for (std::string line; getline(infile, line);)
	{
		for (char& c : line)
		{
			int x = GetTileX(index);
			int y = GetTileY(index);

			bool isWalkable = c == ' ' || c == '.';

			if (isWalkable)
			{
				Tile* tile = new Tile(x, y, isWalkable);
				tileGrid[index] = tile;
				tiles.push_back(tile);
				if (isWalkable) walkableTiles.push_back(tile);
			}

			index += 1;
		}
	}
}

int GridMap::GetTileIndex(int x, int y) const
{
	return x + y * width;
}

int GridMap::GetTileX(int index) const
{
	return index % width;
}

int GridMap::GetTileY(int index) const
{
	return index == 0 ? 0 : static_cast<int>(floor(static_cast<float>(index) / static_cast<float>(width)));
}

Tile* GridMap::GetTileAt(int index) const
{
	bool inBounds = index >= 0 && index < width * height;
	return inBounds ? tileGrid[index] : nullptr;
}

Tile* GridMap::GetTileAt(int x, int y) const
{
	bool inBounds = x >= 0 && x < width;
	return inBounds ? GetTileAt(GetTileIndex(x, y)) : nullptr;
}

Tile* GridMap::GetTileRelativeTo(const Tile* tile, int x, int y)
{
	return GetTileAt(tile->x + x, tile->y + y);
}

bool GridMap::IsWalkable(const int x, const int y) const
{
	Tile* tile = GetTileAt(x, y);
	return tile && tile->isWalkable;
}

Tile* GridMap::RandomWalkableTile()
{
	return walkableTiles[rand() % walkableTiles.size()];
}

void GridMap::SetObstacle(Tile* tile)
{
	if (!tile) return;

	std::cout << "Set " << *tile << " as obstacle " << std::endl;
	tile->isWalkable = false;
	walkableTiles.erase(std::remove(walkableTiles.begin(), walkableTiles.end(), tile), walkableTiles.end());
}
