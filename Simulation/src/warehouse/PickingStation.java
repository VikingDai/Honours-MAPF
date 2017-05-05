package warehouse;

import search.SearchNode;

public class PickingStation
{
    public SearchNode node;

    public PickingStation(SearchNode node)
    {
        this.node = node;
        OrderHandler.RandomOrder();
    }
}

