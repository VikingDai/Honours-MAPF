package warehouse;

import search.SearchNode;

public class StoragePod
{
    public SearchNode homeNode;
    public OrderHandler.OrderType product;

    public StoragePod(SearchNode homeNode)
    {
        this.homeNode = homeNode;
        this.product = OrderHandler.RandomOrder();
    }
}
