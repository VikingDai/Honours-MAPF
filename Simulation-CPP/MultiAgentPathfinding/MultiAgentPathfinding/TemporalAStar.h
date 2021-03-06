#pragma once
#include <vector>
#include <queue>
#include <map>
#include <list>
#include "Timer.h"
#include "Helper.h"
#include <set>
#include "MAPF.h"
#include "PriorityQueue.h"
#include <iostream>

class Tile;
class GridMap;

struct AStarTileTime
{
private:
	AStarTileTime()
	{
		Reset();
	}

public:
	static std::vector<AStarTileTime*> TILE_TIME_POOL;
	static AStarTileTime* Make(std::vector<AStarTileTime*>& usedTileTimes);
	static void FillPool(int num)
	{
		for (int i = 0; i < num; i++)
			TILE_TIME_POOL.push_back(new AStarTileTime());
	};

	bool bClosed = false;
	bool bIsInOpen = false;

	AStarTileTime* parent;

	int timestep = 0;
	Tile* tile;
	

private:
	float f = 0;
	float g = 0;
	float penalty = 0;

public:
	float h = 0;

	float GetAccPenalty() const
	{
		float parentPenalty = parent ? parent->GetPenalty() : 0;
		return penalty + parentPenalty;
	}

	float GetPenalty() const
	{
		//float parentPenalty = parent;// ? parent->GetPenalty() : 0;
		return penalty;// +parentPenalty;
	}

	float GetF() const
	{
		return GetG() + h; //+ GetPenalty();
	}

	float GetG() const
	{
		float parentG = parent ? parent->GetG() : 0;
		return parent ? parent->GetG() + (1 + GetPenalty()) : 0;
	}

	void SetParent(AStarTileTime* parent)
	{
		this->parent = parent;
		//this->g = parent ? parent->g + 1 : 0;
		UpdateCosts();
	}

	void UpdateCosts()
	{
		//float parentG = parent ? parent->g : 0;
		//this->g = parentG + 1 + penalty;
		this->f = GetG() + h;
	}

	void SetInfo(AStarTileTime* parent, int timestep, Tile* tile, float heuristic, float penalty)
	{
		SetParent(parent);

		this->timestep = timestep;
		this->tile = tile;

		this->h = heuristic;
		this->penalty = penalty;

		UpdateCosts();
	}

	void SetPenalty(float penalty)
	{
		this->penalty = penalty;
		UpdateCosts();
	}

	void Reset()
	{
		bClosed = false;
		parent = nullptr;
		bIsInOpen = false;

		parent = nullptr;
		tile = nullptr;
		timestep = f = g = penalty = h = 0;
	}


	int priority = INT_MAX;
	int GetPriority() { return priority; }
	void SetPriority(int priority)
	{
		this->priority = priority;
	}

	bool operator<(const AStarTileTime& other)
	{
		if (GetF() == other.GetF())
		{
			if (GetG() == other.GetG())
			{
				return GetPenalty() < other.GetPenalty();
			}
			return GetG() > other.GetG();
		}

		return GetF() < other.GetF();
	}

	bool operator>(const AStarTileTime& other)
	{
		if (GetF() == other.GetF())
		{
			if (GetG() == other.GetG())
			{
				return GetPenalty() > other.GetPenalty();
			}

			return GetG() < other.GetG();
		}

		return GetF() > other.GetF();
	}

	friend std::ostream& operator<<(std::ostream& os, AStarTileTime& tileTime)
	{
		os << "Tile " << *tileTime.tile <<
			" | time: " << tileTime.timestep <<
			" | f: " << tileTime.GetF() <<
			" | g: " << tileTime.GetG() <<
			" | h: " << tileTime.h <<
			" | penalty: " << tileTime.GetPenalty() <<
			" | acc penalty: " << tileTime.GetAccPenalty();

		os << " | parent: ";
		if (tileTime.parent != nullptr)
			os << "Tile " << *tileTime.parent->tile << " @ " << tileTime.parent->timestep;
		else
			os << "nullptr";

		return os;
	}
};

class TemporalAStar
{
private:
	std::vector<AStarTileTime*> usedTileTimes;
	std::vector<AStarTileTime*> closed;
	bool needsSorting;

public:


	static int GLOBAL_TILES_EXPANDED;
	int LOCAL_TILES_EXPANDED;

public:
	using TileCosts = std::map<int, std::map<Tile*, float>>;

private:
	Timer timer;
	Timer sortTimer;

	GridMap* gridMap;

public:
	TemporalAStar(GridMap* gridMap);
	void SetGridMap(GridMap* gridMap) { this->gridMap = gridMap; }
	~TemporalAStar();

public:
	using OpenQueue = PriorityQueue<AStarTileTime*>;

	std::vector<Tile*> modifiedTiles;
	std::vector<AStarTileTime*> modifiedTileTimes;

	std::map<Tile*, std::map<int, AStarTileTime*>> spatialGridMap;

	MAPF::Path FindPath(Tile* start, Tile* goal, MAPF::CollisionPenalties& penalties = MAPF::CollisionPenalties());

	void ExpandNeighbor(OpenQueue& open, AStarTileTime* current, Tile* neighborTile, Tile* start, Tile* goal, MAPF::CollisionPenalties& penalties);

	int GetCustomCosts(int timestep, Tile* fromTile, Tile* toTile, MAPF::CollisionPenalties& penalties);
};

