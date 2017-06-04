package warehouse;

import expanders.GridMapExpansionPolicy;
import expanders.JPSExpansionPolicy;
import expanders.JPSWarehouseExpansionPolicy;
import expanders.WarehouseExpansionPolicy;
import heuristics.ManhattanHeuristic;
import sample.Agent;
import sample.Main;
import sample.Simulation;
import search.FlexibleAStar;
import search.SearchNode;

import java.util.Collections;
import java.util.Comparator;
import java.util.Optional;
import java.util.stream.Stream;

public class DriveUnit extends Agent
{
    public StoragePod currentPod;
    public OrderHandler.OrderType order;
    public UnitAction action;

    public enum UnitAction
    {
        IDLE,
        GETTING,
        DELIVERING,
        RETURNING,
    }

    public DriveUnit(SearchNode currentNode,
                     FlexibleAStar<ManhattanHeuristic, GridMapExpansionPolicy> search)
    {
        super(currentNode, search);

        action = UnitAction.IDLE;

        expansionPolicy = new JPSExpansionPolicy(Main.getSimulation().map, this);
    }

    // todo replace with state machine
    @Override
    public void OnReachDestination()
    {
        switch (action)
        {
            case IDLE:
                order = OrderHandler.OrderType.CHOCOLATE;// OrderHandler.RandomOrder();
                action = UnitAction.GETTING;

                // next action: find storage pod containing new order
//                Optional<StoragePod> goalStoragePod =

//                Stream<StoragePod> podsWithOrder = Main.getSimulation().storagePods
//                        .stream()
//                        .filter(sp -> sp.product == order);
//
//                Optional<StoragePod> goalStoragePod = podsWithOrder.sorted(
//                        Comparator.comparingInt(sp -> Math.abs(sp.homeNode.x - currentNode.x) + Math.abs(sp.homeNode.y - currentNode.y))).findAny();
////                    .findFirst();
//
//                goalStoragePod.ifPresent(storagePod ->
//                {
//                    currentPod = storagePod;
//                    goalNode = storagePod.homeNode;
//                });

                // random storage pod
                Collections.shuffle(Main.getSimulation().storagePods);
                currentPod = Main.getSimulation().storagePods.get(0);
                goalNode = currentPod.homeNode;


                break;
            case GETTING:
                goalNode.hasStoragePod = false;
                Main.getSimulation().storagePods.remove(currentPod);
//                System.out.println("Delivering storage pod");
                action = UnitAction.DELIVERING;

                // find nearest picking station
                Optional<PickingStation> pickingStations = Main.getSimulation().pickingStations
                        .stream()
                        .sorted(Comparator.comparingInt(ps -> Math.abs(ps.node.x - currentNode.x) + Math.abs(ps.node
                                .y - currentNode.y)))
                        .findFirst();
                pickingStations.ifPresent(pickingStation -> goalNode = pickingStation.node);
                break;
            case DELIVERING:
                System.out.println("Returning storage pod");
                action = UnitAction.RETURNING;
                goalNode = currentPod.homeNode;
                break;
            case RETURNING:
                goalNode.hasStoragePod = true;
                Main.getSimulation().storagePods.add(currentPod);
                System.out.println("Idling for one frame storage pod");
                action = UnitAction.IDLE;
                goalNode = currentNode;
                break;
        }
    }

    @Override
    public void tick(float dt)
    {
        super.tick(dt);
//        System.out.println(action);
    }
}
