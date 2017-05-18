package search;

import expanders.GridMapExpansionPolicy;
import graphics.GUI;
import heuristics.BaseHeuristic;
import javafx.scene.paint.Color;
import sample.Main;
import utils.ISearch;

import java.time.Duration;
import java.time.Instant;
import java.util.*;

public class FlexibleAStar<H extends BaseHeuristic, E extends GridMapExpansionPolicy> implements ISearch
{
    private Map<SearchNode, SearchNode> cameFrom;
    private Queue<SearchNode> open;
    private List<SearchNode> closed;

    private H heuristic;
    private E expander;
    private SearchNode current;
    private int goalId;


    private boolean waitForInput;
    public boolean searching;


    // meta search info
    private int nodesExpanded;
    private int nodesGenerated;
    private int nodesTouched;

    private long totalSearches;
    private long totalSearchTime;

    public FlexibleAStar(H heuristic, E expander)
    {
        closed = new ArrayList<>();
        cameFrom = new HashMap<>();
        open = new PriorityQueue<>(new SearchNodeComparator());
        this.heuristic = heuristic;
        this.expander = expander;
        this.searching = false;
        nodesExpanded = nodesGenerated = nodesTouched = 0;
        totalSearches = totalSearchTime = 0;
    }

    public Stack<SearchNode> findPath(SearchNode start, SearchNode goal)
    {
        Instant startTime = Instant.now();
        Stack<SearchNode> path = new Stack<>();

        initSearch(start, goal);

        while (!open.isEmpty())
        {
            Optional<Stack<SearchNode>> maybePath = step(goal);
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

    public Optional<Stack<SearchNode>> step(SearchNode goal)
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

        expandCurrent(goal);

        return Optional.empty();
    }

    public void expandCurrent(SearchNode goal)
    {
        int costToN = 1; //current.getF();//Integer.MAX_VALUE;

        for (SearchNode node : expander.getNeighbours(current.searchId))
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
                int gVal = current.g + costToN;

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
