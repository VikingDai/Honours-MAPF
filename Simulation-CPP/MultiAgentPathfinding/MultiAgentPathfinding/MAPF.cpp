#include "MAPF.h"

#include "Agent.h"

std::vector<MAPF::AgentPathRef*> MAPF::AgentPathRef::PATH_REF_POOL;

MAPF::AgentPathRef* MAPF::AgentPathRef::Make(Agent* agent, int pathIndex, std::vector<AgentPathRef*>& usedPathRefs)
{
	AgentPathRef* pathRef = nullptr;

	if (PATH_REF_POOL.empty())
	{
		pathRef = new AgentPathRef(agent, pathIndex);
	}
	else
	{
		pathRef = PATH_REF_POOL.back();
		pathRef->agent = agent;
		pathRef->pathIndex = pathIndex;
		PATH_REF_POOL.pop_back();
	}

	usedPathRefs.push_back(pathRef);

	return pathRef;
}

MAPF::AgentPathRef* MAPF::AgentPathRef::MakeNull(Agent* agent)
{
	return new AgentPathRef(agent, -1);
}

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
	os << "Path " << pathRef.pathIndex << ": Agent(" << pathRef.agent->GetAgentId() << ")";
	return os;
}