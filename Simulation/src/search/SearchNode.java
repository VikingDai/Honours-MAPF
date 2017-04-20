package search;

import domains.GridMap;
import javafx.scene.paint.Color;
import javafx.scene.paint.Paint;
import sample.Tile;

public class SearchNode
{
    private int f, g, h;
    private SearchNode parent;
    private int priority;
    private int searchId;
    private boolean hasExpanded;
    private Tile tile;
    private int x, y;

    public SearchNode(int x, int y)
    {
        this.x = x;
        this.y = y;
        parent = null;
        f = g = priority = Integer.MAX_VALUE;
    }

    public int getX()
    {
        return x;
    }

    public int getY()
    {
        return y;
    }

    public void relax(int newG, SearchNode parent)
    {
        assert(newG < g);
        f = (f - g) + newG; // update f
        g = newG;
        this.parent = parent;
    }

    public void setSearchId(GridMap map)
    {
        searchId = map.xyToSearchId(x, y);
    }

    public Tile getTile()
    {
        return tile;
    }

    public void setTile(Tile tile)
    {
        this.tile = tile;
        tile.setSearchNode(this);
    }

    public boolean hasExpanded()
    {
        return hasExpanded;
    }

    public void setHasExpanded(boolean hasExpanded)
    {
        this.hasExpanded = hasExpanded;
    }

    public int getF()
    {
        return f;
    }

    public int getG()
    {
        return g;
    }

    public void updateCost(int g, int h)
    {
        this.g = g;
        this.h = h;
        this.f = g + h;
    }

    public void setParent(SearchNode parent)
    {
        this.parent = parent;
    }
    public void setPriority(int priority)
    {
        this.priority = priority;
    }

    public int getHeuristic()
    {
        return h;
    }

    public SearchNode getParent()
    {
        return parent;
    }

    public int getPriority()
    {
        return priority;
    }

    public int getSearchId()
    {
        return searchId;
    }

    @Override
    public boolean equals(Object o)
    {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        SearchNode that = (SearchNode) o;
        return searchId == that.searchId;
    }

    @Override
    public int hashCode()
    {
        return searchId;
    }

    @Override
    public String toString()
    {
        return "SearchNode{" +
                "x=" + x +
                ", y=" + y +
                '}' +
                " | {f=" + f +
                ", g=" + g +
                ", h=" + h +
                '}';
    }
}
