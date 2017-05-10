package sample;

import domains.GridMap;
import domains.GridMapParser;
import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import search.FlexibleAStar;
import search.ProblemInstance;
import utils.Globals;
import utils.ReservationTable;
import warehouse.DriveUnit;
import warehouse.PickingStation;
import warehouse.StoragePod;

import java.awt.*;
import java.util.ArrayList;
import java.util.List;

public class Simulation
{
    public GridMap map;
    public FlexibleAStar aStar;
    public List<Agent> agents;
    public List<StoragePod> storagePods;
    public List<PickingStation> pickingStations;

    public ReservationTable reservationTable;
    public int timestep;


    public Simulation()
    {
        agents = new ArrayList<>();
        storagePods = new ArrayList<>();
        pickingStations = new ArrayList<>();
    }

    public void init(String mapPath)
    {
        ProblemInstance instance = new ProblemInstance("instances/warehouse1.instance");
        GridMapParser mapParser = new GridMapParser(instance);

        map = new GridMap(mapParser);
        aStar = new FlexibleAStar<>(new ManhattanHeuristic(mapParser.getMetaInfo()), new GridMapExpansionPolicy(map));

        // Add actors to simulation
        for (int i = 0; i < 100; i++)
            agents.add(new DriveUnit(map.getRandomNode(), aStar));

        reservationTable = new ReservationTable();
    }

    public void step()
    {
//        System.out.println("\t### Timestep " + timestep + " ###");
        for (Agent agent : agents)
        {
            agent.step();
//            reservationTable.update(timestep, agent);
        }

//        reservationTable.findConflicts(timestep);
//        System.out.println(timestep);
//        reservationTable.print();


//        System.out.println(reservationTable.table.size());

        agents.forEach(a -> a.tick(0));

        timestep += 1;
    }

    void tick(float dt)
    {
        agents.forEach(a -> a.tick(dt));
    }

    void draw(GraphicsContext gc)
    {
        gc.clearRect(0, 0, Main.CANVAS_MAX_SIZE, Main.CANVAS_MAX_SIZE);

        // DRAW THE IDENTIFIER OF THE AGENTS
        for (Agent agent : agents)
        {
            gc.setStroke(Color.BLACK);
            gc.strokeText(
                    "" + agent.agentId,
                    agent.circle.getCenterX(),
                    agent.circle.getCenterY());
        }

        // DRAW PATHS OF THE AGENTS
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

        // DRAW STORAGE PODS
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

        // DRAW THE PICKING STATIONS
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

    void drawMap(GraphicsContext gc)
    {
        gc.clearRect(0, 0, Main.CANVAS_MAX_SIZE, Main.CANVAS_MAX_SIZE);
        for (Tile tile : map.getTiles())
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
