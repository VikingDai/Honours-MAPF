package search;

import expanders.GridMapExpansionPolicy;
import graphics.GUI;
import heuristics.BaseHeuristic;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import sample.Main;
import utils.DebugPoint;
import utils.Globals;
import utils.ISearch;

import java.time.Duration;
import java.time.Instant;
import java.util.*;

public class FlexibleAStar<H extends BaseHeuristic, E extends GridMapExpansionPolicy>
{
    private Map<SearchNode, SearchNode> cameFrom;
    private Queue<SearchNode> open;
    private List<SearchNode> closed;

    private H heuristic;
    private SearchNode current;

    private boolean waitForInput;
    public boolean searching;


    // meta search info
    private int nodesExpanded;
    private int nodesGenerated;
    private int nodesTouched;

    private long totalSearches;
    private long totalSearchTime;

    public FlexibleAStar(H heuristic)
    {
        closed = new ArrayList<>();
        cameFrom = new HashMap<>();
        open = new PriorityQueue<>(new SearchNodeComparator());
        this.heuristic = heuristic;
        this.searching = false;
        nodesExpanded = nodesGenerated = nodesTouched = 0;
        totalSearches = totalSearchTime = 0;
    }

    public Stack<SearchNode> findPath(SearchNode start, SearchNode goal, E expander)
    {
        Instant startTime = Instant.now();
        Stack<SearchNode> path = new Stack<>();

        initSearch(start, goal);

        while (!open.isEmpty())
        {
            Optional<Stack<SearchNode>> maybePath = step(goal, expander);
            if (maybePath.isPresent())
            {
                path = maybePath.get();
                break;
            }
        }

        long duration = Duration.between(startTime, Instant.now()).toNanos();
        totalSearchTime += duration;
        totalSearches += 1;
        Main.getGUI().setAvgSearch(totalSearchTime / (double) (totalSearches * 1000));

        return path;
    }

    public void initSearch(SearchNode start, SearchNode goal)
    {


        Globals.debugPoints.clear();
//        Globals.debugPoints.add(new DebugPoint(start, Color.BLACK, 0.6));
//        Globals.debugPoints.add(new DebugPoint(goal, Color.BLACK, 0.6));

        // reset tiles from last search
        closed.forEach(n -> n.hasExpanded = false);
        closed.clear();
        cameFrom.clear();
        open.clear();

        // reset stats
        nodesExpanded = nodesGenerated = nodesTouched = 0;

        searching = true;
        start.updateCost(0, heuristic.h(start, goal));

        open.add(start);
    }

    public Optional<Stack<SearchNode>> step(SearchNode goal, E expander)
    {
        if (open.peek().equals(goal)) // found path
        {
            searching = false;

            // rebuild path
            Stack<SearchNode> path = new Stack<>();
            SearchNode current = open.poll();
            path.push(current);

            while (cameFrom.keySet().contains(current))
            {
                current = cameFrom.get(current);
                path.push(current);
            }
            return Optional.of(path);
        }

        nodesExpanded++;
        current = open.poll();
        current.hasExpanded = true;
        closed.add(current);

//        Globals.debugPoints.add(new DebugPoint(current.x, current.y, Color.BLUE, 0.5));

        expandCurrent(goal, expander);

        return Optional.empty();
    }

    public void expandCurrent(SearchNode goal, E expander)
    {
        int costToN = 1; //current.getF();//Integer.MAX_VALUE;

        for (SearchNode node : expander.expand(current.searchId, goal))
        {
            nodesTouched++;
            if (node.hasExpanded)
                continue;

            if (open.contains(node))
            {
                // update a homeNode from the fringe
                int gVal = current.g + costToN;
                if (gVal < node.h)
                {

                    // TODO figure out what this does
                    // open.decreaseKey(n);
//                     n.relax(gVal, current);
                }
            }
            else
            {
                // add node to the fringe
                int gVal = current.g + heuristic.h(current, node);

                cameFrom.put(node, current);

                // update node
                node.updateCost(gVal, heuristic.h(node, goal));
                node.parent = current;


                open.add(node);
                nodesGenerated++;
            }
        }
    }
}
