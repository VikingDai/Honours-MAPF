package search;

import expanders.GridMapExpansionPolicy;
import heuristics.BaseHeuristic;

import java.util.PriorityQueue;
import java.util.Queue;

public class FlexibleAStar<H extends BaseHeuristic, E extends GridMapExpansionPolicy>
{
    private Queue<SearchNode> open;
    private H heuristic;
    private E expander;

    private int nodesExpanded;
    private int nodesGenerated;
    private int nodesTouched;

    public FlexibleAStar(H heuristic, E expander)
    {
        open = new PriorityQueue<>();
        this.heuristic = heuristic;
        this.expander = expander;
    }

    SearchNode search(int x1, int y1, int x2, int y2)
    {
        nodesExpanded = nodesGenerated = nodesTouched = 0;
        ProblemInstance instance = new ProblemInstance(1, 2);

        heuristic.h(x1, y1, x2, y2);
        return null;
    }

}
