package sample;


import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import search.FlexibleAStar;
import search.SearchNode;

import java.time.Duration;
import java.time.Instant;
import java.util.Stack;

public class Agent
{
    public Circle circle;
    public Color color;
    public Stack<SearchNode> path;
    public SearchNode nextNode;
    public SearchNode currentNode;
    boolean reachedNext;

    private FlexibleAStar<ManhattanHeuristic, GridMapExpansionPolicy> search;

    public int agentId;
    private static int agentCounter = 0;

    public Agent(SearchNode currentNode, FlexibleAStar<ManhattanHeuristic, GridMapExpansionPolicy> search)
    {
        agentId = agentCounter;
        agentCounter += 1;

        this.currentNode = currentNode;
        reachedNext = true;
        this.search = search;
        path = new Stack<>();
        color = Color.color(Math.random(), Math.random(), Math.random());

        circle = new Circle(Tile.GRID_SIZE, color);
    }

    public void step()
    {
        if (path.isEmpty()) // search to random position
        {
            SearchNode randomNode = Main.getSimulation().map.getRandomNode();

            Instant startTime = Instant.now();
            path = search.findPath(currentNode.x, currentNode.y, randomNode.x, randomNode.y);
            if (path.size() > 1) // skip first node of the path (where you start)
                path.pop();

            // update reservation table TODO: move this somewhere else
//            Main.getSimulation().reservationTable.update(Main.getSimulation().timestep, this);
//            Main.getSimulation().reservationTable.findConflicts(Main.getSimulation().timestep);

            long time = Duration.between(startTime, Instant.now()).toMillis();
//            System.out.println(" took " + time + " milliseconds");
        }

        nextNode = path.pop();
        currentNode = nextNode;
    }

    public void tick(float dt)
    {
        circle.setRadius(Tile.GRID_SIZE * 0.4);
        circle.setCenterX(currentNode.x * Tile.GRID_SIZE + Tile.GRID_SIZE / 2);
        circle.setCenterY(currentNode.y * Tile.GRID_SIZE + Tile.GRID_SIZE / 2);
    }
}
