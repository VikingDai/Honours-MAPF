package expanders;

import domains.GridMap;
import javafx.scene.paint.Color;
import search.SearchNode;
import utils.DebugPoint;
import utils.Globals;
import warehouse.DriveUnit;

import java.util.List;
import java.util.Optional;

public class JPSExpansionPolicy extends GridMapExpansionPolicy
{
    DriveUnit driveUnit;

    public JPSExpansionPolicy(GridMap map, DriveUnit driveUnit)
    {
        super(map);
        this.driveUnit = driveUnit;
    }

    public Optional<SearchNode> JumpHorizontal(int x, int y, int dx, SearchNode goal)
    {
        Optional<SearchNode> maybeCurrent = map.getSearchNodeAt(x + dx, y);
        while (nodeIsWalkable(maybeCurrent))
        {
            SearchNode current = maybeCurrent.get();
            Globals.debugPoints.add(new DebugPoint(current.x, current.y, Color.BLUE, 0.15));

            if (current == goal)
            {
                return maybeCurrent;
            }

            if (!nodeIsWalkable(map.getSearchNodeRelative(current, -dx, 1)) &&
                    nodeIsWalkable(map.getSearchNodeRelative(current, 0, 1)))
            {
                return maybeCurrent;
            }

            if (!nodeIsWalkable(map.getSearchNodeRelative(current, -dx, -1)) &&
                    nodeIsWalkable(map.getSearchNodeRelative(current, 0, -1)))
            {
                return maybeCurrent;
            }

//            if (!map.getSearchNodeRelative(current, -dx, 1).isPresent() &&
//                    map.getSearchNodeRelative(current, 0, 1).isPresent())
//            {
//                return maybeCurrent;
//            }

//            if (!map.getSearchNodeRelative(current, -dx, -1).isPresent() &&
//                    map.getSearchNodeRelative(current, 0, -1).isPresent())
//            {
//                return maybeCurrent;
//            }

            maybeCurrent = map.getSearchNodeRelative(current, dx, 0);
        }

        return Optional.empty();
    }

    public boolean JumpVertical(int x, int y, int dy, SearchNode goal)
    {
        Optional<SearchNode> maybeCurrent = map.getSearchNodeAt(x, y + dy);
        while (nodeIsWalkable(maybeCurrent))
        {
            SearchNode current = maybeCurrent.get();
            Globals.debugPoints.add(new DebugPoint(current.x, current.y, Color.BLUE, 0.15));

            if (current == goal)
            {
                neighbours.add(current);
                return true;
            }

            if (JumpHorizontal(current.x, current.y, -1, goal).isPresent())
            {
                neighbours.add(current);
                return true;
            }

            if (JumpHorizontal(current.x, current.y, 1, goal).isPresent())
            {
                neighbours.add(current);
                return true;
            }

            maybeCurrent = map.getSearchNodeRelative(current, 0, dy);
        }
        return false;
    }

    public boolean nodeIsWalkable(Optional<SearchNode> maybeNode)
    {
        boolean hasPod = (driveUnit.action == DriveUnit.UnitAction.DELIVERING || driveUnit.action == DriveUnit
                .UnitAction.RETURNING);

        if (maybeNode.isPresent())
        {
            return !(maybeNode.get().hasStoragePod && hasPod);
        }

        return false;
    }

    @Override
    public List<SearchNode> expand(int x, int y, SearchNode goal)
    {
        neighbours.clear();
        Optional<SearchNode> maybeNode = map.getSearchNodeAt(x, y);

        if (!maybeNode.isPresent())
            return neighbours;

        JumpHorizontal(x, y, 1, goal).ifPresent(n -> neighbours.add(n));
        JumpHorizontal(x, y, -1, goal).ifPresent(n -> neighbours.add(n));
        JumpVertical(x, y, 1, goal);
        JumpVertical(x, y, -1, goal);

        System.out.println(neighbours.size());

        return neighbours;
    }
}
