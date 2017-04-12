package search;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

/**
 * Reads in a file in format:
 * type octile
 * height 768
 * width 768
 */
public class GridMapParser
{
    private GridMapMetaInfo header;
    private List<Character> map;

    public GridMapParser(String fileName) throws FileNotFoundException
    {
        loadFile(fileName);
    }

    public boolean loadFile(String fileName)
    {
        Scanner scanner;
        try { scanner = new Scanner(new File(fileName)); }
        catch (FileNotFoundException e) { e.printStackTrace(); return false; }

        map = new ArrayList<>();
        header = new GridMapMetaInfo(scanner); // load meta info

        assert scanner.hasNextLine();

        String mapIdentifier = scanner.nextLine();
        assert mapIdentifier.equals("map"); // map file requires "map" identifier on line #4

        while (scanner.hasNextLine())
        {
            String line = scanner.nextLine();
            System.out.println(line);

            for (char c : line.toCharArray())
                map.add(c);
        }

        return true;
    }

    public GridMapMetaInfo getHeader()
    {
        return header;
    }

    public char getTileAt(int index)
    {
        assert map.size() < index;
        return map.get(index);
    }

    public char getTileAt(int x, int y)
    {
        return getTileAt(x * header.getHeight() +  y);
    }
}

class GridMapMetaInfo
{
    private String mapType;

    private int height, width;

    public GridMapMetaInfo(Scanner scanner)
    {
        assert scanner.hasNextLine(); // map file requires map type identifier on line #1
        mapType = scanner.nextLine().split(" ")[1];

        assert scanner.hasNextLine(); // map file requires height identifier on line #2
        height = Integer.parseInt(scanner.nextLine().split(" ")[1]);

        assert scanner.hasNextLine(); // map file requires width identifier on line #3
        width = Integer.parseInt(scanner.nextLine().split(" ")[1]);

        System.out.println("Loaded map header: " + this);
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
