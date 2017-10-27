#include "MAPF.h"

#include "Agent.h"

std::vector<MAPF::AgentPathRef*> MAPF::AgentPathRef::PATH_REF_POOL;

//MAPF::AgentPathRef* MAPF::AgentPathRef::Make(Agent* agent, int pathIndex, std::vector<AgentPathRef*>& usedPathRefs)
//{
//	AgentPathRef* pathRef = nullptr;
//
//	if (PATH_REF_POOL.empty())
//	{
//		pathRef = new AgentPathRef(agent, pathIndex);
//	}
//	else
//	{
//		pathRef = PATH_REF_POOL.back();
//		pathRef->agent = agent;
//		pathRef->pathIndex = pathIndex;
//		PATH_REF_POOL.pop_back();
//	}
//
//	usedPathRefs.push_back(pathRef);
//
//	return pathRef;
//}
//
//MAPF::AgentPathRef* MAPF::AgentPathRef::MakeNull(Agent* agent)
//{
//	return new AgentPathRef(agent, -1);
//}

MAPF::Path& MAPF::AgentPathRef::GetPath()
{
	if (IsValid())
	{
		return agent->pathBank[pathIndex];
	}
	else
	{
		MAPF::Path emptyPath;
		std::cout << "PATH INVALID: SIZE " << emptyPath.size() << std::endl;
		return emptyPath;
	}
}

bool MAPF::AgentPathRef::IsValid()
{
	return agent && pathIndex >= 0 && pathIndex < agent->pathBank.size();
}

std::ostream& MAPF::operator<<(std::ostream& os, AgentPathRef& pathRef)
{
	os << "a" << pathRef.agent->GetAgentId() << "p" << pathRef.pathIndex << ": {";

	Path& path = pathRef.GetPath();
	for (int i = 0; i < path.size(); i++)
	{
		Tile* tile = path[i];
		os << *tile;

		if (i < path.size() - 1) 
			os << ", ";
	}
	os << "}";
	return os;
}

std::ostream& MAPF::operator<<(std::ostream& os, MAPF::PathCollision& collision)
{
	os << "Collision with Delta " << collision.delta << ", path bank size " << collision.SmallestPathBankSize() << " between:" << std::endl;
	os << "\t" << *collision.a << " (" << collision.a->GetPath().size() << ", " << collision.a->agent->shortestPathLength << ", " << collision.a->agent->delta << ")" << std::endl;
	os << "\t" << *collision.b << " (" << collision.b->GetPath().size() << ", " << collision.b->agent->shortestPathLength << ", " << collision.a->agent->delta << ")" << std::endl;

	return os;
}

MAPF::PathCollision::PathCollision(AgentPathRef* a, AgentPathRef* b) : a(a), b(b)
{
	delta = a->agent->delta + b->agent->delta;
}

int MAPF::PathCollision::SmallestPathBankSize()
{
	return min(a->agent->pathBank.size(), b->agent->pathBank.size());
}
