package warehouse;

import javax.swing.text.html.Option;
import java.util.List;
import java.util.Optional;
import java.util.Queue;
import java.util.stream.Stream;

public class OrderHandler
{
    Queue<ProductType> incomingOrders;

    List<StoragePod> storagePods;

    public enum ProductType
    {
        SHIRT,
        JACKET,
        PANTS,
        CHOCOLATE,
        CHIPS,
        CHAIR,

    }

    public OrderHandler(List<StoragePod> storagePods)
    {
        this.storagePods = storagePods;
    }

    public void Generate()
    {
        ProductType[] orders = ProductType.values();
        incomingOrders.add(orders[(int)(Math.random() * ProductType.values().length)]);
    }

    public StoragePod RequestOrder()
    {
        ProductType order = incomingOrders.poll();
        Optional<StoragePod> storagePod =
                storagePods
                .stream()
                .filter(s -> s.products.contains(order)).findAny();

        return storagePod.orElse(null);
    }

    public void GiveOrder()
    {

    }
}
