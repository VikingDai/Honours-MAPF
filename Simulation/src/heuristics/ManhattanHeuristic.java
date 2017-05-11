package heuristics;

import domains.GridMapMetaInfo;
import search.SearchNode;

public class ManhattanHeuristic extends BaseHeuristic
{
    private int mapWidth;
    private int mapHeight;

    public ManhattanHeuristic(int mapWidth, int mapHeight)
    {
        this.mapWidth = mapWidth;
        this.mapHeight = mapHeight;
    }

    public ManhattanHeuristic(GridMapMetaInfo mapMetaInfo)
    {
        this(mapMetaInfo.getWidth(), mapMetaInfo.getHeight());
    }

    @Override
    public int h(int x1, int y1, int x2, int y2)
    {
        return (Math.abs(x2 - x1) + Math.abs(y2 - y1));
    }

    @Override
    public int h(int startId, int goalId)
    {
        int xS = startId % mapWidth;
        int yS = startId / mapWidth;

        int xE = goalId % mapWidth;
        int yE = goalId / mapWidth;

        return h(xS, yS, xE, yE);
    }

    @Override
    public int h(SearchNode start, SearchNode goal)
    {
        return h(start.x, start.y, goal.x, goal.y);
    }
}
