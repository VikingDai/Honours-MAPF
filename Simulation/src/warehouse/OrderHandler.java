package warehouse;

import java.util.List;
import java.util.Optional;
import java.util.Queue;

public class OrderHandler
{
    Queue<OrderType> incomingOrders;

    List<StoragePod> storagePods;

    public enum OrderType
    {
        SHIRT,
        JACKET,
        PANTS,
        CHOCOLATE,
        CHIPS,
        CHAIR,
        TEDDY_BEAR,
    }

    public OrderHandler(List<StoragePod> storagePods)
    {
        this.storagePods = storagePods;
    }

    public void Generate()
    {
        OrderType[] orders = OrderType.values();
        incomingOrders.add(orders[(int)(Math.random() * OrderType.values().length)]);
    }

    public StoragePod RequestOrder()
    {
        OrderType order = incomingOrders.poll();
        Optional<StoragePod> storagePod =
                storagePods
                .stream()
                .filter(s -> s.product.equals(order)).findAny();

        return storagePod.orElse(null);
    }

    public static OrderType RandomOrder()
    {
        OrderType[] orders = OrderType.values();
        return orders[(int)(Math.random() * OrderType.values().length)];
    }

    public void GiveOrder()
    {

    }
}
