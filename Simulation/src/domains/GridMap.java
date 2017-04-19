package domains;

import sample.Tile;
import search.SearchNode;

import java.util.ArrayList;
import java.util.List;

public class GridMap
{
    private int paddedWidth, paddedHeight, paddingPerRow;
    private int db;
    private SearchNode[] nodes;

    private List<Tile> tiles;

    public int width;
    public int height;

    public GridMap(GridMapParser parser)
    {
        this(parser.getMetaInfo().getWidth(), parser.getMetaInfo().getHeight());

        List<String> map = parser.getMap(); // map as list of strings
        for (int y = 0; y < map.size(); y++)
        {
            char[] chars = map.get(y).toCharArray();
            for (int x = 0; x < chars.length; x++) // split map into chars
            {
                char c = chars[x];

                Tile tile = new Tile(x, y, c == '@' ? Tile.TileType.BLOCKED : Tile.TileType.EMPTY);
                tiles.add(tile);

                if (c != '@') // don't add blocked wall tiles to the search map
                {
                    SearchNode node = new SearchNode(x, y);
                    node.setSearchId(this);
                    setSearchNode(node, x, y);
                    node.setTile(tile);
                }
            }
        }
    }

    public GridMap(int width, int height)
    {
        this.width = width;
        this.height = height;
        nodes = new SearchNode[width * height];
        tiles = new ArrayList<>();
    }

    public void setSearchNode(SearchNode node, int x, int y)
    {
        nodes[x + y * width] = node;
    }

    public int getX(int searchId)
    {
        return searchId % width;
    }

    public int getY(int searchId)
    {
        return searchId / width;
    }

    public int xyToSearchId(int x, int y)
    {
        return x + y * width;
    }

    public SearchNode getSearchNodeAt(int x, int y)
    {
        return nodes[xyToSearchId(x, y)];
    }

    public boolean doesSearchNodeExist(int x, int y)
    {
        return nodes[xyToSearchId(x, y)] != null;
    }

    public boolean checkPositionValid(int x, int y)
    {
        return x >= 0 && y >= 0 && x < width && y < height && doesSearchNodeExist(x, y);
    }

    public List<SearchNode> getNeighbours(int x, int y)
    {
        ArrayList<SearchNode> neighbours = new ArrayList<>();

        if (checkPositionValid(x + 1, y))
            neighbours.add(getSearchNodeAt(x + 1, y));

        if (checkPositionValid(x - 1, y))
            neighbours.add(getSearchNodeAt(x - 1, y));

        if (checkPositionValid(x, y + 1))
            neighbours.add(getSearchNodeAt(x, y + 1));

        if (checkPositionValid(x, y - 1))
            neighbours.add(getSearchNodeAt(x, y - 1));

        return neighbours;
    }

    public List<Tile> getTiles()
    {
        return tiles;
    }
}
