package expanders;

import domains.GridMap;
import sample.Main;
import sample.Simulation;
import search.SearchNode;
import utils.MAPF;
import warehouse.DriveUnit;

import java.util.List;
import java.util.Optional;

public class JPSWarehouseExpansionPolicy extends JPSExpansionPolicy
{
    int relativeTime;
    public JPSWarehouseExpansionPolicy(GridMap map, DriveUnit driveUnit)
    {
        super(map, driveUnit);
        relativeTime = 0;
    }

    @Override
    public Optional<SearchNode> JumpHorizontal(int x, int y, int dx, SearchNode goal)
    {
        relativeTime += 1;
        return super.JumpHorizontal(x, y, dx, goal);
    }

    @Override
    public boolean JumpVertical(int x, int y, int dy, SearchNode goal)
    {
        relativeTime += 1;
        return super.JumpVertical(x, y, dy, goal);
    }

    @Override
    public List<SearchNode> expand(int x, int y, SearchNode goal)
    {
        relativeTime = 0;
        return super.expand(x, y, goal);
    }

    @Override
    public boolean nodeIsWalkable(Optional<SearchNode> maybeNode)
    {
        if (super.nodeIsWalkable(maybeNode))
        {
            if (maybeNode.isPresent())
            {
                SearchNode node = maybeNode.get();
                int timestep = Main.getSimulation().timeStep + node.g + relativeTime;
                return !MAPF.table.isOccupied(timestep, node.x, node.y);
            }
        }

        return false;
    }
}
