package domains;

import search.SearchNode;

import java.util.ArrayList;
import java.util.List;

public class GridMap
{
    private int paddedWidth, paddedHeight, paddingPerRow;
    private int db;
    private SearchNode[] nodes;

    public int width;
    public int height;

    public GridMap(int width, int height)
    {
        this.width = width;
        this.height = height;
        nodes = new SearchNode[width * height];
    }

    public void setSearchNode(SearchNode node, int x, int y)
    {
        nodes[x + y * width] = node;
    }

    public SearchNode getSearchNodeAt(int x, int y)
    {
        return nodes[x + y * width];
    }

    public boolean checkPositionValid(int x, int y)
    {
        return x >= 0 && y >= 0 && x <= width && y <= height;
    }

    public List<SearchNode> getNeighbours(int x, int y)
    {
        ArrayList<SearchNode> neighbours = new ArrayList<>();

        if (checkPositionValid(x + 1, y))
            neighbours.add(getSearchNodeAt(x + 1, y));

        if (checkPositionValid(x - 1, y))
            neighbours.add(getSearchNodeAt(x - 1, y));

        if (checkPositionValid(x, y + 1))
            neighbours.add(getSearchNodeAt(x, y + 1));

        if (checkPositionValid(x, y - 1))
            neighbours.add(getSearchNodeAt(x, y - 1));

        return neighbours;
    }
}
