package warehouse;

import sample.Main;
import sample.Simulation;
import search.SearchNode;
import utils.Globals;

import java.util.List;

public class PickingStation
{
    public SearchNode node;

    public List<SearchNode> neighbours;

    public PickingStation(SearchNode node)
    {
        this.node = node;
    }

    public SearchNode GetRandomNeighbour()
    {
        return Main.getSimulation().map.getSearchNodeAt(
                node.x + Globals.RNG.nextInt(3) - 1,
                node.y + Globals.RNG.nextInt(3) - 1);
}
}

