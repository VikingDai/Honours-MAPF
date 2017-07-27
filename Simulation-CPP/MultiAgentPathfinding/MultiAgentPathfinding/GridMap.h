#pragma once
#include <vector>
#include "Tile.h"
#include <glm/glm.hpp>



class GridMap
{
private:
	std::string mapType;
	int width, height;
	int numTiles;
public:
	const int getNumTiles() { return numTiles; }
	const int getWidth() { return width; }
	const int getHeight() { return height; }

public:
	std::vector<Tile*> walkableTiles;
	std::vector<Tile*> tiles;
	std::vector<Tile*> tileGrid;

	GridMap();
	~GridMap();

	void loadMap(const std::string filename);
	int getTileIndex(int x, int y) const;
	int getTileX(int index) const;
	int getTileY(int index) const;
	Tile* getTileAt(int index) const;
	Tile* getTileAt(int x, int y) const;
	Tile* getTileRelativeTo(const Tile* tile, int x, int y);
	bool isWalkable(int x, int y) const;
	Tile* randomWalkableTile();
};