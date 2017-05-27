package warehouse;

import javafx.scene.paint.Color;
import search.SearchNode;

public class StoragePod
{
    public Color color;
    public SearchNode homeNode;
    public OrderHandler.OrderType product;

    public StoragePod(SearchNode homeNode)
    {
        this.homeNode = homeNode;
        this.product = OrderHandler.RandomOrder();

        switch (product)
        {
            case SHIRT:
                color = Color.PINK;
                break;
            case JACKET:
                color = Color.RED;
                break;
            case PANTS:
                color = Color.GREEN;
                break;
            case CHOCOLATE:
                color = Color.BROWN;
                break;
            case CHIPS:
                color = Color.YELLOW;
                break;
            case CHAIR:
                color = Color.TEAL;
                break;
            case TEDDY_BEAR:
                color = Color.AQUA;
                break;
        }
    }
}
