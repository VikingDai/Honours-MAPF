#pragma once

#include <deque>
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

		friend std::ostream& operator<<(std::ostream& os, AgentPathRef& pathRef);
	};

	using PathCollision = std::vector<AgentPathRef*>;
}