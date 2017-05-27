package utils;

import domains.GridMap;
import heuristics.ManhattanHeuristic;
import sample.Agent;
import search.FlexibleAStar;
import search.SearchNode;

import java.util.List;
import java.util.Stack;

public class MAPF
{
    public ReservationTable table;
    public FlexibleAStar aStar;

    public MAPF(GridMap gridMap)
    {
        table = new ReservationTable();
//        aStar = new FlexibleAStar(new ManhattanHeuristic(gridMap.meta));
    }

    public void UpdatePaths(List<Agent> agents, SearchNode start, SearchNode goal)
    {
        for (Agent agent : agents)
        {
            aStar.findPath(start, goal, null);
        }
    }

    public void FindPath(Agent agent, SearchNode start, SearchNode goal)
    {
        Stack path = aStar.findPath(start, goal, agent.expansionPolicy);
    }
}
