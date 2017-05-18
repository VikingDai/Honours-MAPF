package sample;


import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import search.FlexibleAStar;
import search.SearchNode;
import utils.Globals;

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

        circle = new Circle(Globals.RENDER_SCALE, color);
    }

    public void step()
    {
        if (path.isEmpty()) // reached destination
        {
            OnReachDestination();
            SearchNode nodeToMoveTo = GenerateGoalNode();

            path = search.findPath(currentNode, nodeToMoveTo);
            if (path.size() > 1) // skip first homeNode of the path (where you start)
                path.pop();
            else
                return; // failed to find a path
        }

        nextNode = path.pop();
        currentNode = nextNode;
    }

    public void OnReachDestination()
    {

    }

    public SearchNode GenerateGoalNode()
    {
        return Main.getSimulation().map.getRandomNode();
    }

    public void tick(float dt)
    {
        circle.setRadius(Globals.RENDER_SCALE * 0.4);
        circle.setCenterX(currentNode.x * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2);
        circle.setCenterY(currentNode.y * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2);
    }
}
