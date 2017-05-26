package warehouse;

import sample.Main;
import search.SearchNode;
import utils.Globals;

import java.util.List;

public class PickingStation
{
    public PickingStation closestPickingStation;
    public float distToPickingStation;

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
                node.y + Globals.RNG.nextInt(3) - 1).orElse(null);
    }

    public void UpdateClosestPickingStation(PickingStation closestStation) // todo update the closest picking station
    {
        closestPickingStation = closestStation;
        distToPickingStation = Math.abs(closestStation.node.x - node.x) + Math.abs(closestStation.node.y - node.y);
    }
}

