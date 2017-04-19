package search;

import expanders.GridMapExpansionPolicy;
import heuristics.BaseHeuristic;
import javafx.scene.Node;
import javafx.scene.paint.Color;
import javafx.scene.shape.Path;

import java.util.*;


public class FlexibleAStar<H extends BaseHeuristic, E extends GridMapExpansionPolicy>
{
    private Map<SearchNode, SearchNode> cameFrom;
    private Queue<SearchNode> open;
    private H heuristic;
    private E expander;
    private SearchNode current;
    private int goalId;

    private int goalX, goalY;

    private int nodesExpanded;
    private int nodesGenerated;
    private int nodesTouched;

    private boolean waitForInput;

    public boolean searching;

    public FlexibleAStar(H heuristic, E expander)
    {
        cameFrom = new HashMap<>();
        open = new PriorityQueue<>(new SearchNodeComparator());
        this.heuristic = heuristic;
        this.expander = expander;
        this.searching = false;
    }

    public Queue<SearchNode> initSearch(int startX, int startY, int goalX, int goalY)
    {
        cameFrom.clear();
        open.clear();

        searching = true;

        Queue<SearchNode> path = new PriorityQueue<>();
        this.goalX = goalX;
        this.goalY = goalY;

        expander.generate(goalX, goalY).getTile().setFill(Color.RED);

        nodesExpanded = nodesGenerated = nodesTouched = 0;
//        ProblemInstance instance = new ProblemInstance(1, 2);
//        instance.setGoal(goalId);
//        instance.setStart(startX, startY);
//        instance.setSearchId(); // TODO: What is this initSearch id for

        SearchNode goal = null;
        SearchNode start = expander.generate(startX, startY);
        start.updateCost(0, heuristic.h(startX, startY, goalX, goalY)); //* hscale);

        open.add(start);

        System.out.println("Searching");

        return path;
    }

    public void step()
    {
        nodesTouched++;
        if (open.peek().getX() == goalX && open.peek().getY() == goalY) // found path
        {
            searching = false;

            // rebuild path
            System.out.println("Found goal!");
            SearchNode current = open.poll();
            Stack<SearchNode> path = new Stack<>();
            path.push(current);


            while (cameFrom.keySet().contains(current))
            {

                current = cameFrom.get(current);
                path.push(current);
            }

            path.forEach(p -> p.getTile().setFill(Color.TEAL));
//            goal = open.peek(); // TODO whats this for?
//            break;
        }

        nodesExpanded++;
        current = open.poll();
        current.setHasExpanded(true);

        System.out.println("Expanding: " + current);
//        expander.expand(current, instance);

        expandCurrent();

        System.out.println("Open list size: " + open.size());
    }

    public void expandCurrent()
    {
        int costToN = current.getF();//Integer.MAX_VALUE;

        for (SearchNode n : expander.getNeighbours(current.getSearchId()))
        {
            nodesTouched++;
            if (n.hasExpanded())
                continue;

            if (open.contains(n))
            {
                // update a node from the fringe
                int gVal = current.getF() + costToN;
                if (gVal < n.getHeuristic())
                {

                    // TODO figure out what this does
                    // n.relax(gVal, current);
                    // open.decreaseKey(n);
                }
            }
            else
            {
                // add a new node to the fringe
                int gVal = current.getG() + 1;
                cameFrom.put(n, current);
                n.updateCost(gVal, heuristic.h(n.getX(), n.getY(), goalX, goalY));
                n.setParent(current);
                n.getTile().setFill(Color.PINK);
                open.add(n);
                nodesGenerated++;
            }
        }
    }



}
