package expanders;

import domains.GridMap;
import search.ProblemInstance;
import search.SearchNode;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public class GridMapExpansionPolicy
{
    protected GridMap map;
    protected List<SearchNode> neighbours = new ArrayList<>();

    public GridMapExpansionPolicy(GridMap map)
    {
        this.map = map;
        neighbours = new ArrayList<>();
    }

    public SearchNode generate(int x, int y)
    {
        return map.getSearchNodeAt(x, y).orElse(null);
    }

    public List<SearchNode> getNeighbours(int searchId)
    {
        return getNeighbours(map.getX(searchId), map.getY(searchId));
    }

    public List<SearchNode> getNeighbours(int x, int y)
    {
        neighbours.clear();
        Optional<SearchNode> maybeNode = map.getSearchNodeAt(x, y);

        if (!maybeNode.isPresent())
            return neighbours;

        if (map.checkPositionValid(x + 1, y))
            map.getSearchNodeAt(x + 1, y).ifPresent(neighbours::add);

        if (map.checkPositionValid(x - 1, y))
            map.getSearchNodeAt(x - 1, y).ifPresent(neighbours::add);

        if (map.checkPositionValid(x, y + 1))
            map.getSearchNodeAt(x, y + 1).ifPresent(neighbours::add);

        if (map.checkPositionValid(x, y - 1))
            map.getSearchNodeAt(x, y - 1).ifPresent(neighbours::add);

        return neighbours;
    }

    public void expand(SearchNode current, ProblemInstance instance)
    {

    }

    public List<SearchNode> expand(int searchId, SearchNode goal)
    {
        return expand(map.getX(searchId), map.getY(searchId), goal);
    }

    public List<SearchNode> expand(int x, int y, SearchNode goal)
    {
        return new ArrayList<>();
    }
}
