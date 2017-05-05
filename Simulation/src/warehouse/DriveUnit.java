package warehouse;

import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import sample.Agent;
import sample.Main;
import search.FlexibleAStar;
import search.SearchNode;

import java.util.Collections;
import java.util.Comparator;
import java.util.Optional;

public class DriveUnit extends Agent
{
    public StoragePod currentPod;
    public OrderHandler.OrderType order;
    private SearchNode goalNode;

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

        goalNode = currentNode;

        action = UnitAction.IDLE;
    }

    // todo replace with state machine
    @Override
    public void OnReachDestination()
    {
        switch (action)
        {
            case IDLE:
                order = OrderHandler.RandomOrder();
//                System.out.println("Getting storage pod");
                action = UnitAction.GETTING;

//                // next action: find storage pod containing new order
//                Optional<StoragePod> goalStoragePod = Main.getSimulation().storagePods
//                        .stream()
//                        .filter(sp -> sp.product == order)
//                        .sorted(Comparator.comparingInt(sp -> sp.homeNode.x - currentNode.x + sp.homeNode.y - currentNode.y))
//                        .findFirst();
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
                Main.getSimulation().storagePods.remove(currentPod);
//                System.out.println("Delivering storage pod");
                action = UnitAction.DELIVERING;


                // find nearest picking station
//                Optional<PickingStation> pickingStations = Main.getSimulation().pickingStations
//                        .stream()
//
//                        .sorted(Comparator.comparingInt(ps -> ps.node.x - currentNode.x + ps.node.y - currentNode.y))
//                        .findFirst();
//                pickingStations.ifPresent(pickingStation -> goalNode = pickingStation.node);

                // random picking station
                Collections.shuffle(Main.getSimulation().pickingStations);
                goalNode = Main.getSimulation().pickingStations.get(0).node;


                break;
            case DELIVERING:
//                System.out.println("Returning storage pod");
                action = UnitAction.RETURNING;
                goalNode = currentPod.homeNode;
                break;
            case RETURNING:
                Main.getSimulation().storagePods.add(currentPod);
//                System.out.println("Idling for one frame storage pod");
                action = UnitAction.IDLE;
                goalNode = currentNode;
                break;
        }
    }

    @Override
    public SearchNode GenerateGoalNode()
    {
        return goalNode;
//        if (!holdingStoragePod) // find closest storage pod containing order
//        {
//            Optional<StoragePod> goalStoragePod = Main.getSimulation().storagePods
//                    .stream()
//                    .filter(sp -> sp.product == order)
//                    .sorted(Comparator.comparingInt(sp -> sp.homeNode.x - currentNode.x + sp.homeNode.y - currentNode.y))
//                    .findFirst();
//
//            if (goalStoragePod.isPresent())
//            {
//                currentPod = goalStoragePod.get();
//                return currentPod.homeNode;
//            }
//
//            System.out.println("Failed to find homeNode containing order: " + order.name());
//            return currentNode;
//        }
//        else // find closest picking station
//        {
//            Optional<PickingStation> pickingStations = Main.getSimulation().pickingStations
//                    .stream()
//                    .sorted(Comparator.comparingInt(ps -> ps.node.x - currentNode.x + ps.node.y - currentNode.y))
//                    .findFirst();
//
//            if (pickingStations.isPresent())
//            {
//                return pickingStations.get().node;
//            }
//
//            System.err.println("Failed to find any picking stations!");
//            return currentNode;
//        }
    }
}
