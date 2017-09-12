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
	const int GetNumTiles() { return numTiles; }
	const int GetWidth() { return width; }
	const int GetHeight() { return height; }

public:
	std::vector<Tile*> walkableTiles;
	std::vector<Tile*> tiles;
	std::vector<Tile*> tileGrid;

	GridMap();
	~GridMap();

	int GetTileIndex(int x, int y) const;
	int GetTileX(int index) const;
	int GetTileY(int index) const;
	Tile* GetTileAt(int index) const;
	Tile* GetTileAt(int x, int y) const;
	Tile* GetTileRelativeTo(const Tile* tile, int x, int y);
	bool IsWalkable(int x, int y) const;
	Tile* RandomWalkableTile();

	void SetObstacle(Tile* tile);

protected:
	void LoadMap(const std::string filename);

	friend class Environment;
};