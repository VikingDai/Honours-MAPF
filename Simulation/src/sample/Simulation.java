package sample;

import domains.GridMap;
import domains.GridMapParser;
import expanders.GridMapExpansionPolicy;
import graphics.GUI;
import graphics.GUI.RenderLayer;
import heuristics.ManhattanHeuristic;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import search.FlexibleAStar;
import search.ProblemInstance;
import search.SearchNode;
import utils.Globals;
import utils.ReservationTable;
import warehouse.DriveUnit;
import warehouse.PickingStation;
import warehouse.StoragePod;
import warehouse.Warehouse;

import java.awt.*;
import java.util.ArrayList;
import java.util.List;

public class Simulation
{
    public SearchNode start;
    public SearchNode goal;

    public GridMap map;
    public FlexibleAStar aStar;
    public List<Agent> agents;
    public List<StoragePod> storagePods;
    public List<PickingStation> pickingStations;

    public ReservationTable reservationTable;
    public int timeStep;

    public Warehouse warehouse;
    public boolean isTicking;

    public Simulation()
    {
        isTicking = false;
        agents = new ArrayList<>();
        storagePods = new ArrayList<>();
        pickingStations = new ArrayList<>();
        warehouse = new Warehouse();
    }

    public void init(String problemInstancePath)
    {
        ProblemInstance instance = new ProblemInstance("instances/warehouse1.instance");
        GridMapParser mapParser = new GridMapParser(instance);

        map = new GridMap(mapParser);
        aStar = new FlexibleAStar<>(new ManhattanHeuristic(mapParser.getMetaInfo()), new GridMapExpansionPolicy(map));

        // add actors to simulation
//        for (int i = 0; i < 5; i++)
            agents.add(new DriveUnit(map.getRandomNode(), aStar));

        reservationTable = new ReservationTable();
    }

    public void step(GraphicsContext gc)
    {
        for (Agent agent : agents)
        {
            agent.step();
        }

//        agents.forEach(a -> a.tick(0));

        // update rendering
        draw(gc);
        drawCollisions(gc);

        // update time step
        timeStep += 1;
        Main.getGUI().setTimeStep(timeStep);
    }

    void tick(float dt)
    {
        if (isTicking)
            step(Main.getGUI().getLayer(RenderLayer.MOVING));

        agents.forEach(a -> a.tick(dt));
    }

    void draw(GraphicsContext gc)
    {
        gc.clearRect(0, 0, GUI.CANVAS_MAX_SIZE, GUI.CANVAS_MAX_SIZE);

        // draw the agent id
        for (Agent agent : agents)
        {
            gc.setStroke(Color.BLACK);
            gc.strokeText(
                    "" + agent.agentId,
                    agent.circle.getCenterX(),
                    agent.circle.getCenterY());
        }

        // draw the agent paths
        gc.setStroke(Color.BLACK);
        gc.setLineWidth(2f);
        Point start;
        Point end;
        gc.setStroke(Color.BLACK);

        for (Agent agent : agents)
        {
            for (int i = 1; i < agent.path.size() + 1; i++)
            {
                start = agent.path.get(i - 1).tile.tilePos;

                if (i == agent.path.size())
                    end = agent.currentNode.tile.tilePos;
                else
                    end = agent.path.get(i).tile.tilePos;

                gc.strokeLine(
                        start.x * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2,
                        start.y * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2,
                        end.x * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2,
                        end.y * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2);
            }
        }

        // draw the storage pods
        gc.setLineWidth(0);
        gc.setFill(Color.GREEN);
        for (StoragePod pod : storagePods)
        {
            gc.fillRect(
                    pod.homeNode.x * Globals.RENDER_SCALE,
                    pod.homeNode.y * Globals.RENDER_SCALE,
                    Globals.RENDER_SCALE * 0.8,
                    Globals.RENDER_SCALE * 0.8);
        }

        // draw the picking stations
        gc.setLineWidth(0);
        gc.setFill(Color.RED);
        for (PickingStation station : pickingStations)
        {
            gc.fillRect(
                    station.node.x * Globals.RENDER_SCALE,
                    station.node.y * Globals.RENDER_SCALE,
                    Globals.RENDER_SCALE * 0.8,
                    Globals.RENDER_SCALE * 0.8);
        }
    }

    public void drawMap()
    {
        GraphicsContext gc = Main.getGUI().getLayer(RenderLayer.BACKGROUND);
        // draw the tiles
        gc.clearRect(0, 0, GUI.CANVAS_MAX_SIZE, GUI.CANVAS_MAX_SIZE);
        for (Tile tile : map.tiles)
        {
            gc.setFill(tile.getFill());
            gc.fillRect(
                    tile.tilePos.getX() * Globals.RENDER_SCALE,
                    tile.tilePos.getY() * Globals.RENDER_SCALE,
                    Globals.RENDER_SCALE * 0.95,
                    Globals.RENDER_SCALE * 0.95);
        }
    }

    public void drawCollisions(GraphicsContext gc)
    {
        // mark any collisions with a circle
        gc.setFill(Color.RED);
        for (List<Point> positionList : reservationTable.collisions.values())
        {
            for (Point collision : positionList)
            {
                gc.fillOval(
                        collision.x * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2 - Globals.RENDER_SCALE * 0.125,
                        collision.y * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2 - Globals.RENDER_SCALE * 0.125,
                        Globals.RENDER_SCALE * 0.25,
                        Globals.RENDER_SCALE * 0.25);
            }
        }
    }
}
