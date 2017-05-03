package heuristics;

import domains.GridMapMetaInfo;

public class EuclideanHeuristic extends BaseHeuristic
{
    private int mapWidth;
    private int mapHeight;

    public EuclideanHeuristic(int mapWidth, int mapHeight)
    {
        this.mapWidth = mapWidth;
        this.mapHeight = mapHeight;
    }

    public EuclideanHeuristic(GridMapMetaInfo mapMetaInfo)
    {
        this(mapMetaInfo.getWidth(), mapMetaInfo.getHeight());
    }

    @Override
    public int h(int x1, int y1, int x2, int y2)
    {
        double dx = x2 - x1;
        double dy = y2 - y1;
        return (int) Math.sqrt(dx * dx + dy * dy);
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
}
