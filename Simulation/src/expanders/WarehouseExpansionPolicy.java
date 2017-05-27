package expanders;

import domains.GridMap;
import search.SearchNode;
import warehouse.DriveUnit;

import java.util.List;
import java.util.Optional;

public class WarehouseExpansionPolicy extends GridMapExpansionPolicy
{
    DriveUnit driveUnit;

    public WarehouseExpansionPolicy(GridMap map, DriveUnit driveUnit)
    {
        super(map);
        this.driveUnit = driveUnit;
    }

    @Override
    public List<SearchNode> getNeighbours(int x, int y)
    {
        neighbours.clear();
        Optional<SearchNode> maybeNode = map.getSearchNodeAt(x, y);

        boolean hasPod = (driveUnit.action == DriveUnit.UnitAction.DELIVERING || driveUnit.action == DriveUnit.UnitAction.RETURNING);

        if (!maybeNode.isPresent())
            return neighbours;

        if (map.checkPositionValid(x + 1, y))
            map.getSearchNodeAt(x + 1, y).ifPresent(n ->
            {
                if (!(n.hasStoragePod && hasPod))
                    neighbours.add(n);
            });

        if (map.checkPositionValid(x - 1, y))
            map.getSearchNodeAt(x - 1, y).ifPresent(n ->
            {
                if (!(n.hasStoragePod && hasPod))
                    neighbours.add(n);
            });

        if (map.checkPositionValid(x, y + 1))
            map.getSearchNodeAt(x, y + 1).ifPresent(n ->
            {
                if (!(n.hasStoragePod && hasPod))
                    neighbours.add(n);
            });

        if (map.checkPositionValid(x, y - 1))
            map.getSearchNodeAt(x, y - 1).ifPresent(n ->
            {
                if (!(n.hasStoragePod && hasPod))
                    neighbours.add(n);
            });

        return this.neighbours;
    }
}
