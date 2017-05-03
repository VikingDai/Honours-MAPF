package domains;

import sample.Tile;

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
    private GridMapMetaInfo metaInfo;
    private List<String> mapAsString;

    public GridMapParser(String fileName)
    {
        loadFile(fileName);
    }

    public boolean loadFile(String fileName)
    {
        Scanner scanner;
        try
        {
            scanner = new Scanner(new File(fileName));
        } catch (FileNotFoundException e)
        {
            e.printStackTrace();
            return false;
        }

        mapAsString = new ArrayList<>();
        metaInfo = new GridMapMetaInfo(scanner); // load meta info

        assert scanner.hasNextLine();

        String mapIdentifier = scanner.nextLine();
        assert mapIdentifier.equals("map"); // map file requires "map" identifier on line #4

        while (scanner.hasNextLine())
        {
            String line = scanner.nextLine();
            System.out.println(line);
            mapAsString.add(line);
        }

        return true;
    }

    public List<Tile> createTiles(Tile[][] tiles)
    {
        List<Tile> outList = new ArrayList<>();

        for (int y = 0; y < mapAsString.size(); y++)
        {
            char[] chars = mapAsString.get(y).toCharArray();
            for (int x = 0; x < chars.length; x++)
            {
                char c = chars[x];
                Tile tile = new Tile(x, y, c == '@' ? Tile.TileType.BLOCKED : Tile.TileType.EMPTY);
                tiles[x][y] = tile;
                outList.add(tile);
            }
        }

        return outList;
    }

    public GridMapMetaInfo getMetaInfo()
    {
        return metaInfo;
    }

    public List<String> getMap()
    {
        return mapAsString;
    }
}

