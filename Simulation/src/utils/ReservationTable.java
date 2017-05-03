package utils;

import sample.Agent;
import search.SearchNode;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ReservationTable
{
    // (x, y, t) => List<Agent>
    private Map<Integer, Map<Integer, Map<Integer, List<Agent>>>> table;

    public ReservationTable()
    {
        this.table = new HashMap<>();
    }

    public void updateTable(List<Agent> agents)
    {
        for (Agent agent : agents)
        {
            ArrayList<SearchNode> path = new ArrayList<>(agent.path);
            
        }
    }
}
