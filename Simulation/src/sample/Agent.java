package sample;


import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import javafx.scene.paint.Color;
import search.FlexibleAStar;
import search.SearchNode;

import java.time.Duration;
import java.time.Instant;
import java.util.Stack;

public class Agent
{
    public Color color;
    public Stack<SearchNode> path;
    public SearchNode nextNode;
    public SearchNode currentNode;
    boolean reachedNext;

    private FlexibleAStar<ManhattanHeuristic, GridMapExpansionPolicy> search;

    public Agent(SearchNode currentNode, FlexibleAStar<ManhattanHeuristic, GridMapExpansionPolicy> search)
    {
        this.currentNode = currentNode;
        reachedNext = true;
        this.search = search;
        path = new Stack<>();
        color = Color.color(Math.random(), Math.random(), Math.random());
    }

    public void step()
    {
        if (path.isEmpty()) // search to random position
        {
            SearchNode randomNode = Main.getSimulation().getMap().getRandomNode();

            Instant startTime = Instant.now();
            path = search.findPath(currentNode.getX(), currentNode.getY(), randomNode.getX(), randomNode.getY());
            long time = Duration.between(startTime, Instant.now()).toMillis();
            System.out.println(" took " + time + " milliseconds");
        }

        nextNode = path.pop();
        currentNode = nextNode;
    }
}
