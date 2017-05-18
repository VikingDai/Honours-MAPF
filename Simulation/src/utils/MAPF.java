package utils;

import sample.Agent;
import search.FlexibleAStar;
import search.SearchNode;

import java.util.List;

public class MAPF
{
    public ReservationTable table;
    public FlexibleAStar aStar;

    public MAPF()
    {
        table = new ReservationTable();
    }

    public void UpdatePaths(List<Agent> agents, SearchNode start, SearchNode goal)
    {
        for (Agent agent : agents)
        {
            aStar.findPath(start, goal);
        }


    }

}
