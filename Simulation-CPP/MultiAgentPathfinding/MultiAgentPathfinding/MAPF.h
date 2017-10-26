#pragma once

#include <deque>
#include <unordered_set>
#include <set>
#include "Tile.h"

class Agent;

namespace MAPF
{
	using Path = std::deque<Tile*>;
	using TileTime = std::pair<Tile*, int>;

	static void PrintPath(Path& path)
	{
		std::cout << "Path {";
		for (int i = 0; i < path.size(); i++)
		{
			std::cout << *path[i];

			if (i != path.size() - 1)
				std::cout << ", ";
		}
		std::cout << "}" << std::endl;
	}

	struct AgentPathRef
	{
		static std::vector<AgentPathRef*> PATH_REF_POOL;

		Agent* agent;
		int pathIndex;

	public:
		AgentPathRef(Agent* agent, int pathIndex) : agent(agent), pathIndex(pathIndex) {}

	public:
		//static AgentPathRef* Make(Agent* agent, int pathIndex, std::vector<AgentPathRef*>& usedPathRefs = std::vector<AgentPathRef*>());
		//static AgentPathRef* MakeNull(Agent* agent);

		Path& GetPath();
		bool IsValid();

		friend bool operator==(const AgentPathRef& a, const AgentPathRef& b)
		{
			return a.agent == b.agent && a.pathIndex == b.pathIndex;
		}

		friend std::ostream& operator<<(std::ostream& os, AgentPathRef& pathRef);
	};

	struct AgentPathRefHash
	{
		std::size_t operator()(const AgentPathRef& pathRef) const
		{
			return 1;// std::hash<int>()((int) pathRef.agent + pathRef.pathIndex);
		}
	};

	using PathCollisions = std::vector<std::set<AgentPathRef*>>;//std::vector<std::set<AgentPathRef*, AgentPathRefHash>>;

	using Edge = std::pair<Tile*, Tile*>;

	struct PathCollision
	{
		AgentPathRef* a;
		AgentPathRef* b;

		PathCollision(AgentPathRef* a, AgentPathRef* b) : a(a), b(b) {}

		float CalculateDelta();
		int SmallestPathBankSize();

		friend bool operator==(const PathCollision& a, const PathCollision& b)
		{
			return (a.a == b.a && a.b == b.a) || (a.a == b.b && a.b == b.a);
		}

		friend bool operator<(PathCollision& a, PathCollision& b)
		{
			return (a.a == b.a && a.b == b.a) || (a.a == b.b && a.b == b.a);
		}
	};

	struct PathCollisionHash
	{
		std::size_t operator()(const PathCollision& collision) const
		{
			return std::hash<int>()(int(collision.a) + int(collision.b));
		}
	};

	struct DeltaComparator
	{
		bool operator() (PathCollision& a, PathCollision& b)
		{
			float deltaA = a.CalculateDelta();
			float deltaB = b.CalculateDelta();

			// if delta is the same, then use the collision which has the smaller alternative paths
			if (deltaA == deltaB)
			{
				return a.SmallestPathBankSize() < b.SmallestPathBankSize();
			}

			return a.CalculateDelta() < b.CalculateDelta();

		}
	};
}