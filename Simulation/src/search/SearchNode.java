package search;

import domains.GridMap;
import sample.Tile;


public class SearchNode
{
    public boolean hasStoragePod;
    public int f, g, h;
    public SearchNode parent;
    public int searchId;
    public boolean hasExpanded;
    public Tile tile;
    public int x, y;
    public int pathLength;

    public SearchNode(int x, int y)
    {
        this.x = x;
        this.y = y;

        reset();
//        f = g = Integer.MAX_VALUE;
//        parent = null;
//        hasStoragePod = false;
//        pathLength = -1;
    }

    public void reset()
    {
        f = g = Integer.MAX_VALUE;
        parent = null;
        hasExpanded = false;
        pathLength = -1;
    }

    public void setSearchId(GridMap map)
    {
        searchId = map.xyToSearchId(x, y);
    }

    public void setTile(Tile tile)
    {
        this.tile = tile;
        tile.setSearchNode(this);
    }

    public void updateCost(int g, int h)
    {
        this.g = g;
        this.h = h;
        this.f = g + h;
    }

    public void relax(int newG, SearchNode parent)
    {
        assert (newG < g);
        f = (f - g) + newG;
        g = newG;
        this.parent = parent;
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
