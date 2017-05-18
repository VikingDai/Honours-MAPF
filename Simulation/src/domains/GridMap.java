package domains;

import sample.Main;
import sample.Tile;
import search.SearchNode;
import utils.Globals;
import warehouse.PickingStation;
import warehouse.StoragePod;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public class GridMap
{
    private int paddedWidth, paddedHeight, paddingPerRow;
    private int db;
    private List<SearchNode> nodePool;
    private SearchNode[] nodes;

    public List<Tile> tiles;
    private Tile[][] tilesXY;

    public int width;
    public int height;

    public GridMap(GridMapParser parser)
    {
        this(parser.getMetaInfo().getWidth(), parser.getMetaInfo().getHeight());
        nodePool = new ArrayList<>();
        List<String> map = parser.getMap(); // map as list of strings
        for (int y = 0; y < map.size(); y++)
        {
            char[] chars = map.get(y).toCharArray();
            for (int x = 0; x < chars.length; x++) // split map into chars
            {
                char c = chars[x];

                Tile tile = new Tile(x, y, c == '@' || c == 'S' ? Tile.TileType.BLOCKED : Tile.TileType.EMPTY);
                tiles.add(tile);
                tilesXY[x][y] = tile;

                if (c != '@') // don't add walls to the search tree
                {
                    SearchNode node = new SearchNode(x, y);
                    node.setSearchId(this);
                    setSearchNode(node, x, y);
//                    System.out.println("Added search homeNode at: " + x + " " + y);
                    node.setTile(tile);

                    if (c == 'S') // load storage pods
                    {
                        node.hasStoragePod = true;
                        StoragePod pod = new StoragePod(node);
//                        Simulation sim = Main.getSimulation();
                        Main.getSimulation().storagePods.add(pod);
                    }
                    else if (c == 'P') // spawn picking station
                    {
                        PickingStation station = new PickingStation(node);
                        Main.getSimulation().pickingStations.add(station);
                    }
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
        tilesXY = new Tile[width][height];
    }

    public void setSearchNode(SearchNode node, int x, int y)
    {
        nodes[x + y * width] = node;
        nodePool.add(node);
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

    public Optional<SearchNode> getSearchNodeAt(int x, int y)
    {
        if (checkPositionValid(x, y))
            return Optional.of(nodes[xyToSearchId(x, y)]);

        return Optional.empty();
    }

    public boolean doesSearchNodeExist(int x, int y)
    {
        return nodes[xyToSearchId(x, y)] != null;
    }

    public boolean checkPositionValid(int x, int y)
    {
        return x >= 0 && y >= 0 && x < width && y < height && doesSearchNodeExist(x, y);
    }

    public boolean checkPositionValid(int x, int y, boolean hasStoragePod)
    {
        if (hasStoragePod)
        {
            Optional<SearchNode> maybeNode = getSearchNodeAt(x, y);
            return maybeNode.isPresent() && !maybeNode.get().hasStoragePod;
        }
        else
            return checkPositionValid(x, y);
    }

    public List<SearchNode> getNeighbours(SearchNode node)
    {
        return getNeighbours(node.x, node.y);
    }

    public List<SearchNode> getNeighbours(int x, int y)
    {
        ArrayList<SearchNode> neighbours = new ArrayList<>();
        Optional<SearchNode> maybeNode = getSearchNodeAt(x, y);

        if (!maybeNode.isPresent())
            return neighbours;

        boolean hasStoragePod = maybeNode.get().hasStoragePod;

        if (checkPositionValid(x + 1, y, hasStoragePod))
            getSearchNodeAt(x + 1, y).ifPresent(neighbours::add);

        if (checkPositionValid(x - 1, y, hasStoragePod))
            getSearchNodeAt(x - 1, y).ifPresent(neighbours::add);

        if (checkPositionValid(x, y + 1, hasStoragePod))
            getSearchNodeAt(x, y + 1).ifPresent(neighbours::add);

        if (checkPositionValid(x, y - 1, hasStoragePod))
            getSearchNodeAt(x, y - 1).ifPresent(neighbours::add);

        return neighbours;
    }

    public SearchNode getRandomNode()
    {
        return nodePool.get(Globals.RNG.nextInt(nodePool.size()));
    }
}
