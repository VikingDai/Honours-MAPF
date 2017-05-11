package search;

import expanders.GridMapExpansionPolicy;
import heuristics.BaseHeuristic;
import javafx.scene.paint.Color;

import java.util.*;


public class FlexibleAStar<H extends BaseHeuristic, E extends GridMapExpansionPolicy>
{
    private Map<SearchNode, SearchNode> cameFrom;
    private Queue<SearchNode> open;
    private List<SearchNode> closed;

    private H heuristic;
    private E expander;
    private SearchNode current;
    private int goalId;

    private int nodesExpanded;
    private int nodesGenerated;
    private int nodesTouched;

    private boolean waitForInput;

    public boolean searching;

    // meta search info
    public static int numSearches;

    public FlexibleAStar(H heuristic, E expander)
    {
        closed = new ArrayList<>();
        cameFrom = new HashMap<>();
        open = new PriorityQueue<>(new SearchNodeComparator());
        this.heuristic = heuristic;
        this.expander = expander;
        this.searching = false;
    }

    public Stack<SearchNode> findPath(SearchNode start, SearchNode goal)
    {
        numSearches += 1;

        initSearch(start, goal);

        while (!open.isEmpty())
        {
            Stack<SearchNode> path = step(goal);
            if (path != null) return path;
        }

        return null;
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

    public Stack<SearchNode> step(SearchNode goal)
    {
        // found path
        if (open.peek().equals(goal))
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
            return path;
        }

        nodesExpanded++;
        current = open.poll();
        current.hasExpanded = true;
        closed.add(current);

        expandCurrent(goal);

        return null;
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
                node.updateCost(gVal, heuristic.h(node, goal));
                node.parent = current;
                open.add(node);
                nodesGenerated++;
            }
        }
    }
}
