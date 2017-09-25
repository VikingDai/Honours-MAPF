#include "CentralizedAStar.h"
#include "Heuristics.h"

CentralizedAStar::CentralizedAStar(GridMap* gridMap)
	: gridMap(gridMap)
{

}

void CentralizedAStar::AssignPaths(std::vector<Agent*>& agents)
{
	OpenQueue open;
	closed.clear();

	/** add initial node to open */
	Node* initialNode = new Node(nullptr);
	closed.push_back(initialNode);
	for (Agent* agent : agents)
	{
		initialNode->AddAgent(agent, gridMap->GetTileAt(agent->x, agent->y));
		initialNode->CalculateEstimate(nullptr);
	}

	open.push_back(initialNode);
	//open.push(initialNode);

	while (!open.empty())
	{
		std::sort(open.begin(), open.end(), TieBreaker());
		Node* currentNode = open.back();
		open.pop_back();

		/*Node* currentNode = open.top();
		open.pop();*/

		std::cout << "Expanded node with est: " << currentNode->estimate << std::endl;
		for (auto& it : currentNode->agentTilePos)
		{
			Tile* tile = it.second;
			Agent* agent = it.first;

			//std::cout << "\t" << *agent << " | " << *tile << std::endl;
		}

		//for (Node* node : open)
		//{
			//std::cout << "Est: " << node->estimate << std::endl;
		//}


		if (currentNode->isGoalNode())
		{
			std::cout << "FOUND GOAL!" << std::endl;

			std::map<Agent*, MAPF::Path> agentPath;
			while (currentNode->parent != nullptr)
			{
				for (auto& it : currentNode->agentTilePos)
				{
					Agent* agent = it.first;
					Tile* tile = it.second;

					agentPath[agent].push_front(tile);
				}

				currentNode = currentNode->parent;
			}

			for (auto& it : agentPath)
			{
				it.first->potentialPaths.push_back(it.second);

				std::cout << *it.first << std::endl;

				for (Tile* tile : it.second)
					std::cout << "\t" << *tile << std::endl;
			}

			

			break;
		}

		std::vector<Node*> nodes;
		ExpandNode(currentNode, nodes, agents, AgentActions());
		std::cout << "Created " << nodes.size() << " nodes" << std::endl;

		/*for (Node* node : nodes)
			open.push(node);
*/
		open.insert(open.begin(), nodes.begin(), nodes.end());
	}
}

void CentralizedAStar::ExpandNode(Node* current, std::vector<Node*>& nodes, std::vector<Agent*> agents, AgentActions actions)
{
	/*for (auto& it : actions)
	{
		std::cout << *it.first << " , " << it.second << std::endl;
	}

	std::cout << "---------------" << std::endl;*/

	if (!agents.empty())
	{
		Agent* agent = agents.back();
		agents.pop_back();

		for (int i = 0; i < 5; i++)
		{
			actions[agent] = i;
			ExpandNode(current, nodes, agents, actions);
		}
	}
	else
	{
		Node* node = new Node(current);
		//std::cout << "New Node:" << std::endl;

		bool anyInvalid = false;
		bool anyNonZero = false;

		std::vector<Tile*> agentTiles;

		for (auto& it : actions)
		{
			Agent* agent = it.first;
			Tile* currentTile = current->GetAgentTile(agent);
			Tile* newTile = nullptr;
			switch (it.second)
			{
			case 0: // wait
				newTile = currentTile;
				break;
			case 1: // left
				newTile = gridMap->GetTileRelativeTo(currentTile, -1, 0);
				break;
			case 2: // up
				newTile = gridMap->GetTileRelativeTo(currentTile, 0, 1);
				break;
			case 3: // right
				newTile = gridMap->GetTileRelativeTo(currentTile, 1, 0);
				break;
			case 4: // down
				newTile = gridMap->GetTileRelativeTo(currentTile, 0, -1);
				break;
			}

			anyNonZero = anyNonZero || it.second != 0;

			if (!newTile)
			{
				anyInvalid = true;
				break;
			}

			newTile->SetColor(sf::Color(newTile->GetColor().r + 5, newTile->GetColor().g, newTile->GetColor().b, newTile->GetColor().a));

			/** check for tile collisions */
			if (std::find(agentTiles.begin(), agentTiles.end(), currentTile) != agentTiles.end())
			{
				anyInvalid = true;
				break;
			}

			/** check for cross collisions */
			for (auto& it : actions)
			{
				Agent* otherAgent = it.first;
				if (agent == otherAgent) continue;

				Tile* myPrev = current->GetAgentTile(agent);
				Tile* myNext = newTile;
				Tile* otherPrev = current->GetAgentTile(otherAgent);
				Tile* otherNext = node->GetAgentTile(otherAgent);

				if (otherNext == nullptr) continue;

				//std::cout << *myPrev << ", " << *myNext << " | " << *otherPrev << ", " << *otherNext << std::endl;

				if (myNext == otherPrev && myPrev == otherNext)
				{
					anyInvalid = true;
					break;
				}
			}

			agentTiles.push_back(currentTile);
			node->AddAgent(agent, newTile);

			//std::cout << "\t" << *agent << " at " << *newTile << " | Action: " <<it.second << std::endl;
		}

		if (anyInvalid || !anyNonZero) return;

		node->CalculateEstimate(node);
		nodes.push_back(node);

		closed.push_back(node);
	}
}

void CentralizedAStar::Node::CalculateEstimate(Node* parent)
{
	/* heuristic is calculated per agent and summed up */
	heuristic = 0;
	for (auto& it : agentTilePos)
	{
		Agent* agent = it.first;
		Tile* tile = it.second;
		float heur = Heuristics::Manhattan(tile, agent->goal);
		heuristic += heur;
	}

	cost = parent ? parent->cost + agentTilePos.size() : 0;

	estimate = cost + heuristic;
}
