package sample;

import domains.GridMap;
import domains.GridMapParser;
import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import search.FlexibleAStar;

public class Simulation
{
    private GridMap map;
    private FlexibleAStar aStar;

    private int startX, startY, endX, endY;

    public Simulation()
    {
        startX = startY = endX = endY = 0;

        GridMapParser mapParser = new GridMapParser("maps/newMap.map");
//        GridMapParser mapParser = new GridMapParser("maps/AcrosstheCape.map");

        map = new GridMap(mapParser);

        aStar = new FlexibleAStar<>(new ManhattanHeuristic(mapParser.getMetaInfo()), new GridMapExpansionPolicy(map));
//        aStar.initSearch(1, 10, 10, 1);
    }

    public void step()
    {
        if (!aStar.searching)
            aStar.initSearch(startX, startY, endX, endY);

        aStar.step();
    }

    void update(float dt)
    {

    }

    public GridMap getMap()
    {
        return map;
    }

    public FlexibleAStar getSearch()
    {
        return aStar;
    }

    public void setStart(int x, int y)
    {
        startX = x; startY = y;
    }

    public void setEnd(int x, int y)
    {
        endX = x; endY = y;
    }

}
