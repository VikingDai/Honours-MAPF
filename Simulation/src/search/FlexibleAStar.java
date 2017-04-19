package search;

import expanders.GridMapExpansionPolicy;
import heuristics.BaseHeuristic;
import javafx.scene.paint.Color;

import java.util.PriorityQueue;
import java.util.Queue;

import static java.lang.Thread.sleep;

public class FlexibleAStar<H extends BaseHeuristic, E extends GridMapExpansionPolicy>
{
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

    public FlexibleAStar(H heuristic, E expander)
    {
        open = new PriorityQueue<>(new SearchNodeComparator());
        this.heuristic = heuristic;
        this.expander = expander;

    }

    public SearchNode search(int startX, int startY, int goalX, int goalY)
    {
        this.goalX = goalX;
        this.goalY = goalY;

        expander.generate(goalX, goalY).getTile().setFill(Color.RED);

        nodesExpanded = nodesGenerated = nodesTouched = 0;
//        ProblemInstance instance = new ProblemInstance(1, 2);
//        instance.setGoal(goalId);
//        instance.setStart(startX, startY);
//        instance.setSearchId(); // TODO: What is this search id for
        heuristic.h(startX, startY, goalX, goalY);

        SearchNode goal = null;
//        SearchNode start = expander.generate(startId);
        SearchNode start = expander.generate(startX, startY);
//        start.setG(0);
        start.setF(0, heuristic.h(startX, startY, goalX, goalY)); //* hscale);

        open.add(start);

        System.out.println("Searching");

//        while (!open.isEmpty())
//        {
//            System.out.println("Open Not Empty");
//            nodesTouched++;
//            if (open.peek().getSearchId() == goalId)
//            {
//                System.out.println("Found goal!");
//                goal = open.peek(); // TODO whats this for?
//                break;
//            }
//
//            nodesExpanded++;
//            current = open.poll();
//            current.setHasExpanded(true);
//            expander.expand(current, instance);
//
//            expandCurrent();
//        }

        return goal;
    }

    public void step()
    {
        nodesTouched++;
        if (open.peek().getX() == goalX && open.peek().getY() == goalY)
        {
            System.out.println("Found goal!");
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
                int gVal = current.getHeuristic() + costToN;
//                n.setG(gVal);
//                n.setH(heuristic.h(n.getSearchId(), goalId));
                n.setF(gVal, heuristic.h(n.getX(), n.getY(), goalX, goalY));
                n.setParent(current);
                n.getTile().setFill(Color.PINK);
                open.add(n);
                nodesGenerated++;
            }
        }
    }


}
