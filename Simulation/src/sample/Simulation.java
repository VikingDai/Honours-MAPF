package sample;

import domains.GridMap;
import domains.GridMapParser;
import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import search.FlexibleAStar;

import java.awt.*;
import java.util.ArrayList;
import java.util.List;

public class Simulation
{
    public GridMap map;
    public FlexibleAStar aStar;
    public List<Agent> agents;

    public Simulation()
    {
        init("maps/newMap.map");
    }

    public void init(String mapPath)
    {
        GridMapParser mapParser = new GridMapParser(mapPath);
//        GridMapParser mapParser = new GridMapParser("maps/maze512-1-8.map");
//        GridMapParser mapParser = new GridMapParser("maps/AcrosstheCape.map");

        map = new GridMap(mapParser);
        aStar = new FlexibleAStar<>(new ManhattanHeuristic(mapParser.getMetaInfo()), new GridMapExpansionPolicy(map));

        // Add actors to simulation
        agents = new ArrayList<>();
        for (int i = 0; i < 50; i++)
            agents.add(new Agent(map.getRandomNode(), aStar));
    }

    public void step()
    {
        agents.forEach(Agent::step);
    }

    void tick(float dt)
    {
        agents.forEach(a -> a.tick(dt));
    }

    void drawTiles(GraphicsContext gc)
    {
        gc.setLineWidth(0);
        for (Tile tile : map.getTiles())
        {
            gc.setFill(tile.getFill());
            gc.fillRect(
                    tile.tilePos.getX() * Tile.GRID_SIZE,
                    tile.tilePos.getY() * Tile.GRID_SIZE,
                    Tile.GRID_SIZE * 0.95,
                    Tile.GRID_SIZE * 0.95);
        }
    }

    void drawAgents(GraphicsContext gc)
    {
        gc.setGlobalAlpha(1);
//        for (Agent agent : agents)
//        {
//            gc.setStroke(Color.BLACK);
//
//            gc.setFill(agent.color);
//            gc.fillOval(
//                    agent.currentNode.getTile().tilePos.getX() * Tile.GRID_SIZE,
//                    agent.currentNode.getTile().tilePos.getY() * Tile.GRID_SIZE,
//                    Tile.GRID_SIZE * 0.95,
//                    Tile.GRID_SIZE * 0.95);
//        }
    }

    public void drawPaths(GraphicsContext gc)
    {
        gc.setGlobalAlpha(0.1);
        gc.setStroke(Color.BLACK);
        gc.setLineWidth(2f);

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
}
