package domains;

import java.util.Scanner;

public class GridMapMetaInfo
{
    private String mapType;

    private int height, width;

    public GridMapMetaInfo(Scanner scanner)
    {
        assert scanner.hasNextLine(); // mapAsString file requires mapAsString type identifier on line #1
        mapType = scanner.nextLine().split(" ")[1];

        assert scanner.hasNextLine(); // mapAsString file requires height identifier on line #2
        height = Integer.parseInt(scanner.nextLine().split(" ")[1]);

        assert scanner.hasNextLine(); // mapAsString file requires width identifier on line #3
        width = Integer.parseInt(scanner.nextLine().split(" ")[1]);

        System.out.println("Loaded mapAsString metaInfo: " + this);
    }

    public int getHeight()
    {
        return height;
    }

    public int getWidth()
    {
        return width;
    }

    public String getMapType()
    {
        return mapType;
    }

    @Override
    public String toString()
    {
        return "GridMapMetaInfo {" +
                "mapType='" + mapType + '\'' +
                ", height=" + height +
                ", width=" + width +
                '}';
    }
}
