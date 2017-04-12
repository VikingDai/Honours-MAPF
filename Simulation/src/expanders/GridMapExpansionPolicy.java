package expanders;

import domains.GridMap;
import search.SearchNode;

import java.util.ArrayList;

public class GridMapExpansionPolicy
{
    private GridMap map;
    private ArrayList<SearchNode> nodepool;
    private int which; // current neighbour
    private SearchNode[] neighbours;
    int numNeighbours;

    public SearchNode generate(int x, int y)
    {
         return map.getNeighbours(x, y).get(0);
//        return nodepool();
    }
}
