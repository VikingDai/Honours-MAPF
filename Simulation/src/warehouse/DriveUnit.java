package warehouse;

import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import sample.Agent;
import search.FlexibleAStar;
import search.SearchNode;

public class DriveUnit extends Agent
{
    public StoragePod pod;

    public DriveUnit(SearchNode currentNode, FlexibleAStar<ManhattanHeuristic, GridMapExpansionPolicy> search)
    {
        super(currentNode, search);
    }
}
