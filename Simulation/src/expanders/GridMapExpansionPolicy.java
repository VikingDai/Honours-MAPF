package expanders;

import domains.GridMap;
import search.ProblemInstance;
import search.SearchNode;

import java.util.ArrayList;
import java.util.List;

public class GridMapExpansionPolicy
{
    private GridMap map;
    private ArrayList<SearchNode> nodepool;
    private int which; // current neighbour
    private SearchNode[] neighbours;
    int numNeighbours;

    public GridMapExpansionPolicy(GridMap map)
    {
        this.map = map;
    }

    public SearchNode generate(int x, int y)
    {
        return map.getSearchNodeAt(x, y);
//         return map.getNeighbours(x, y).get(0);
//        return nodepool();
    }

    public List<SearchNode> getNeighbours(int searchId)
    {
        return getNeighbours(map.getX(searchId), map.getY(searchId));
    }

    public List<SearchNode> getNeighbours(int x, int y)
    {
        return map.getNeighbours(x, y);
    }

    public void expand(SearchNode current, ProblemInstance instance)
    {

    }
}
