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


	struct AgentPathRef
	{
		static std::vector<AgentPathRef*> PATH_REF_POOL;

		Agent* agent;
		int pathIndex;

	private:
		AgentPathRef(Agent* agent, int pathIndex) : agent(agent), pathIndex(pathIndex) {}

	public:
		static AgentPathRef* Make(std::vector<AgentPathRef*>& usedPathRefs, Agent* agent, int pathIndex);
		static AgentPathRef* MakeNull(Agent* agent);

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

	struct PathCollision
	{
		AgentPathRef* a;
		AgentPathRef* b;

		PathCollision(AgentPathRef* a, AgentPathRef* b) : a(a), b(b) {}

		float CalculateDelta() { return 1; };

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
			return a.CalculateDelta() < b.CalculateDelta();
		}
	};
}