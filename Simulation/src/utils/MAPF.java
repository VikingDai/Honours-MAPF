package utils;

import domains.GridMapMetaInfo;
import expanders.GridMapExpansionPolicy;
import expanders.JPSExpansionPolicy;
import heuristics.ManhattanHeuristic;
import sample.Agent;
import search.FlexibleAStar;
import search.SearchNode;

import java.util.List;
import java.util.Stack;

public class MAPF
{
    public static ReservationTable table = new ReservationTable();
    public static FlexibleAStar aStar;

    public MAPF(GridMapMetaInfo mapMetaInfo)
    {
        table = new ReservationTable();
        aStar = new FlexibleAStar<>(new ManhattanHeuristic(mapMetaInfo));
    }

    public static void init(GridMapMetaInfo mapMetaInfo)
    {
        table = new ReservationTable();
        aStar = new FlexibleAStar<>(new ManhattanHeuristic(mapMetaInfo));
    }

    public static Stack<SearchNode> FindPath(Agent agent, SearchNode start, SearchNode goal)
    {
        int i = 0;
        while (i < 10)
        {
            Stack<SearchNode> path = aStar.findPath(start, goal, agent.expansionPolicy);
            if (!path.empty())
            {
                Stack<SearchNode> pathCopy = new Stack<>();
                pathCopy.addAll(path);
                table.addPath(agent, pathCopy);
                return path;
            }
        }

        return new Stack<>();
    }

    public static void Update(int timestep)
    {
        table.update(timestep);
    }

//    public static void FindPath(Agent agent, SearchNode start, SearchNode goal)
//    {
//        Stack<SearchNode> path = aStar.findPath(start, goal, agent.expansionPolicy);
//    }
}
