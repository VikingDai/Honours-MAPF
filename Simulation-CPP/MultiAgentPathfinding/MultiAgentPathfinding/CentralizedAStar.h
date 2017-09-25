#pragma once

#include <vector>

#include "Agent.h"

class CentralizedAStar
{
	struct Node
	{
		Node(Node* parent) : parent(parent)
		{
			cost = heuristic = estimate = 0;
		}

		std::map<Agent*, Tile*> agentTilePos;
		Node* parent;
		float heuristic;
		float cost;
		float estimate;

		bool isGoalNode()
		{
			for (auto& it : agentTilePos)
			{
				Agent* agent = it.first;
				Tile* tile = it.second;

				if (agent->goal != tile)
					return false;
			}

			return true;
		}

		void CalculateEstimate(Node* node);
		void AddAgent(Agent* agent, Tile* tile)
		{
			agentTilePos[agent] = tile;
		};

		Tile* GetAgentTile(Agent* agent) { return agentTilePos[agent]; }

		friend bool operator==(const Node &n1, const Node &n2)
		{
			return n1.agentTilePos == n2.agentTilePos;
		}
	};

	struct TieBreaker
	{
	public:
		TieBreaker() = default;
		bool operator()(Node* A, Node* B)
		{
			if (A->estimate == B->estimate)
				return A->cost < B->cost;

			return A->estimate > B->estimate;
		};
	};

	using AgentActions = std::map<Agent*, int>;
	using OpenQueue = std::vector<Node*>;
	//using OpenQueue = std::priority_queue<Node*, std::vector<Node*>, Heuristic>;

	//OpenQueue open;
	std::vector<Node*> closed;

public:
	GridMap* gridMap;

	CentralizedAStar(GridMap* gridMap);

	void AssignPaths(std::vector<Agent*>& agents);
	void ExpandNode(Node* current, std::vector<Node*>& nodes, std::vector<Agent*> agents, AgentActions actions);
};