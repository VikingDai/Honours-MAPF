package warehouse;

import java.util.ArrayList;
import java.util.List;

public class Warehouse
{
    public List<DriveUnit> driveUnits;
    public List<StoragePod> storagePods;
    public List<PickingStation> pickingStations;

    public Warehouse()
    {
        driveUnits = new ArrayList<>();
        storagePods = new ArrayList<>();
        pickingStations = new ArrayList<>();
    }
}
