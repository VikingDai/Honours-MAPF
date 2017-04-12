package search;

public class SearchNode
{
    private int f, g;
    private SearchNode parent;
    private int priority;
    private int search_id;

    public SearchNode(int id)
    {
        parent = null;
        search_id = 0;
        f = g = priority = Integer.MAX_VALUE;
    }

    public int getPathCost()
    {
        return f;
    }

    public int getHeuristic()
    {
        return g;
    }

    public SearchNode getParent()
    {
        return parent;
    }

    public int getPriority()
    {
        return priority;
    }

    public int getSearch_id()
    {
        return search_id;
    }
}
