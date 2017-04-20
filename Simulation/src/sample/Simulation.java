package sample;

import domains.GridMap;
import domains.GridMapParser;
import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import javafx.geometry.Point2D;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import javafx.scene.shape.StrokeLineCap;
import javafx.scene.shape.StrokeLineJoin;
import search.FlexibleAStar;
import search.SearchNode;

import java.awt.*;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.Stack;

public class Simulation
{
    private GridMap map;
    private FlexibleAStar aStar;

    private int startX, startY, endX, endY;

    private List<Agent> agents;

    public Simulation()
    {
        startX = startY = endX = endY = 0;

        GridMapParser mapParser = new GridMapParser("maps/newMap.map");
//        GridMapParser mapParser = new GridMapParser("maps/maze512-1-8.map");

        map = new GridMap(mapParser);
        aStar = new FlexibleAStar<>(new ManhattanHeuristic(mapParser.getMetaInfo()), new GridMapExpansionPolicy(map));

        SearchNode start = map.getRandomNode();
        setStart(start.getX(), start.getY());
        SearchNode end = map.getRandomNode();
        setEnd(end.getX(), end.getY());

        agents = new ArrayList<>();
        for (int i = 0; i < 10; i++)
        {
            agents.add(new Agent(map.getRandomNode(), aStar));
        }

//        aStar.initSearch(1, 10, 10, 1);
    }

    public void step()
    {
//        if (!aStar.searching)
//            aStar.initSearch(startX, startY, endX, endY);

//        aStar.step();
//        System.out.println("Finding path");
//        Instant startTime = Instant.now();
//
//        SearchNode start = map.getRandomNode();
//        setStart(start.getX(), start.getY());
//        SearchNode end = map.getRandomNode();
//        setEnd(end.getX(), end.getY());
//        aStar.findPath(startX, startY, endX, endY);
//
//        long time = Duration.between(startTime, Instant.now()).toMillis();
//        System.out.println("Search took: " + time + " milliseconds");

        agents.forEach(a -> a.step());
    }

    void update(float dt)
    {
//        if (!aStar.searching)
//            aStar.initSearch(startX, startY, endX, endY);
//
//        aStar.step();

        agents.forEach(a -> a.update(dt));
    }

    void drawTiles(GraphicsContext gc)
    {
        for (Tile tile : getMap().getTiles())
        {
            gc.setFill(tile.getFill());
            gc.fillRect(tile.tilePos.getX() * Tile.GRID_SIZE, tile.tilePos.getY() * Tile.GRID_SIZE, Tile.GRID_SIZE, Tile.GRID_SIZE);
        }
    }

    void drawAgents(GraphicsContext gc)
    {
        gc.setGlobalAlpha(1);
        for (Agent agent : agents)
        {
            gc.setLineWidth(1f);
            gc.setStroke(Color.BLACK);

            gc.setFill(agent.getFill());
            gc.fillOval(agent.currentNode.getTile().tilePos.getX() * Tile.GRID_SIZE,
                    agent.currentNode.getTile().tilePos.getY() * Tile.GRID_SIZE,
                    Tile.GRID_SIZE,
                    Tile.GRID_SIZE);

            gc.strokeOval(
                    agent.currentNode.getTile().tilePos.getX() * Tile.GRID_SIZE,
                    agent.currentNode.getTile().tilePos.getY() * Tile.GRID_SIZE,
                    Tile.GRID_SIZE,
                    Tile.GRID_SIZE);
        }
    }

    public void drawPaths(GraphicsContext gc)
    {
        gc.setStroke(Color.BLACK);
        gc.setLineWidth(2f);
        gc.setLineDashes(1, 1, 1);
        gc.setLineCap(StrokeLineCap.ROUND);
        gc.setLineJoin(StrokeLineJoin.BEVEL);
        gc.setGlobalAlpha(0.25f);

        for (Agent agent : agents)
        {
            for (int i = 1; i < agent.path.size(); i++)
            {
                Point start = agent.path.get(i - 1).getTile().tilePos;
                Point end = agent.path.get(i).getTile().tilePos;

                gc.strokeLine(
                        start.x * Tile.GRID_SIZE + Tile.GRID_SIZE / 2,
                        start.y * Tile.GRID_SIZE + Tile.GRID_SIZE / 2,
                        end.x * Tile.GRID_SIZE + Tile.GRID_SIZE / 2,
                        end.y * Tile.GRID_SIZE + Tile.GRID_SIZE / 2);
            }
        }
    }

    public GridMap getMap()
    {
        return map;
    }

    public FlexibleAStar getSearch()
    {
        return aStar;
    }

    public List<Agent> getAgents()
    {
        return agents;
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
