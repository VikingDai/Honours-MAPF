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

    private int goalX, goalY;

    private int nodesExpanded;
    private int nodesGenerated;
    private int nodesTouched;

    private boolean waitForInput;

    public boolean searching;

    public FlexibleAStar(H heuristic, E expander)
    {
        closed = new ArrayList<>();
        cameFrom = new HashMap<>();
        open = new PriorityQueue<>(new SearchNodeComparator());
        this.heuristic = heuristic;
        this.expander = expander;
        this.searching = false;
    }

    public Stack<SearchNode> findPath(int startX, int startY, int goalX, int goalY)
    {
        initSearch(startX, startY, goalX, goalY);

        while (!open.isEmpty())
        {
            Stack<SearchNode> path = step();
            if (path != null)
            {

                System.out.println(
                        "Expanded: " + nodesExpanded +
                                " | Generated: " + nodesGenerated +
                                " | Touched: " + nodesTouched);

                return path;
            }
        }

        return null;
    }

    public void initSearch(int startX, int startY, int goalX, int goalY)
    {
        closed.forEach(n ->
        {
            n.getTile().setFill(Color.WHITE);
            n.getTile().SetType(n.getTile().tileType);
            n.setHasExpanded(false);
//            System.out.println("Resetting node");
        });
        closed.clear();

        cameFrom.clear();
//        open.forEach(n -> n.getTile().SetType(n.getTile().tileType));
        open.clear();

        searching = true;

        this.goalX = goalX;
        this.goalY = goalY;

//        expander.generate(goalX, goalY).getTile().setFill(Color.RED);

        nodesExpanded = nodesGenerated = nodesTouched = 0;
//        ProblemInstance instance = new ProblemInstance(1, 2); // TODO: What is problem instance for
//        instance.setGoal(goalId);
//        instance.setStart(startX, startY);
//        instance.setSearchId(); //

        SearchNode goal = null;
        SearchNode start = expander.generate(startX, startY);
        start.updateCost(0, heuristic.h(startX, startY, goalX, goalY)); //* hscale);

        open.add(start);

        System.out.println("Finding path from {" + startX + ", " + startY + "} to {" + goalX + ", " + goalY + "}");
    }

    public Stack<SearchNode> step()
    {
        nodesTouched++;
        if (open.peek().x == goalX && open.peek().y == goalY) // found path
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

//            path.forEach(p -> p.getTile().setFill(Color.TEAL));

            return path;
//            goal = open.peek(); // TODO whats this for?
//            break;
        }

        nodesExpanded++;
        current = open.poll();
        current.setHasExpanded(true);
//        current.getTile().setFill(Color.AQUA);



        closed.add(current);

//        System.out.println("Expanding: " + current);
//        expander.expand(current, instance);

        expandCurrent();

//        System.out.println("Open list size: " + open.size());

        return null;
    }

    public void expandCurrent()
    {
        int costToN = 1; //current.getF();//Integer.MAX_VALUE;

        for (SearchNode n : expander.getNeighbours(current.searchId))
        {
            nodesTouched++;
            if (n.hasExpanded())
                continue;

            if (open.contains(n))
            {
                // update a node from the fringe
                int gVal = current.g + costToN;
                if (gVal < n.h)
                {

                    // TODO figure out what this does
//                     n.relax(gVal, current);
                    // open.decreaseKey(n);
                }
            }
            else
            {
                // add a new node to the fringe
                int gVal = current.g + costToN;
                cameFrom.put(n, current);
                n.updateCost(gVal, heuristic.h(n.x, n.y, goalX, goalY));
                n.parent = current;
//                n.getTile().setFill(Color.RED);
                open.add(n);
                nodesGenerated++;
            }
        }
    }



}
