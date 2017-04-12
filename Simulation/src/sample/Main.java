package sample;

import com.sun.javafx.geom.Vec2f;
import javafx.animation.Animation;
import javafx.animation.KeyFrame;
import javafx.animation.Timeline;
import javafx.application.Application;
import javafx.event.ActionEvent;
import javafx.print.*;
import javafx.scene.Group;
import javafx.scene.Scene;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.ScrollEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.paint.Color;
import javafx.stage.Stage;
import javafx.util.Duration;
import search.GridMapParser;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.*;


public class Main extends Application
{
    private class ActionComparator implements Comparator<Tile>
    {
        @Override
        public int compare(Tile a1, Tile a2)
        {
            double f1 = CalculateCost(a1) + CalculateHeuristic(a1);
            double f2 = CalculateCost(a2) + CalculateHeuristic(a2);

            if (f1 > f2) return 1;
            if (f2 > f1) return -1;
            return 0;
        }
    }

    private Map<Tile, Tile> cameFrom = new HashMap<>();
    private List<Tile> allTiles = new ArrayList<>();
    private Tile[][] tiles;
    static Tile start = null;
    static Tile goal = null;

    private Vec2f mouseClicked = new Vec2f();
    private boolean hasClicked = false;

    private Stage stage;
    private BorderPane root;
    private Group centerGroup;
    private Scene scene;

    private List<Tile> explored = new ArrayList<>();
    private Queue<Tile> frontier = new PriorityQueue<>(new ActionComparator());

    private boolean tickSearch = false;
    private boolean searchOver = false;

    @Override
    public void start(Stage stage) throws Exception
    {
        new GridMapParser("maps/newMap.map");
//        Parent centerGroup = FXMLLoader.load(getClass().getResource("sample.fxml"));
        double width = 1280;
        double height = 720;


        centerGroup = new Group();
        centerGroup.setOnMouseDragged(this::OnDragged);
        centerGroup.setOnMouseReleased(e -> hasClicked = false);

//        root = new BorderPane();
//        root.setCenter(centerGroup);


//        scene = new Scene(root, width, height);
        scene = new Scene(centerGroup, width, height);

        this.stage = stage;
        stage.setTitle("Warehouse Automation");
        stage.setScene(scene);
        stage.sizeToScene();
        stage.show();

        scene.setOnKeyPressed(this::OnKeyPressed);
        scene.setOnKeyReleased(this::OnKeyReleased);
        scene.setOnScroll(this::OnScrolled);

        // Read in map
        LoadMap("maps/newMap.map");
//        LoadMap("maps/AcrosstheCape.map");


        Timeline timeline = new Timeline(new KeyFrame(Duration.millis(1), this::OnTick));
        timeline.setCycleCount(Animation.INDEFINITE);
        timeline.play();

        UpdateVisibleTiles();
    }

    private void OnTick(ActionEvent e)
    {
        if (tickSearch) UpdateSearch();
    }

    private void LoadMap(String pathname) throws FileNotFoundException
    {
        Scanner sc = new Scanner(new File(pathname));
        List<String> lines = new ArrayList<>();
        int count = 0;
        int maxX = 0;
        int maxY = 0;

        while (sc.hasNextLine())
        {
            if (count < 4)
            {
                String line = sc.nextLine();
//                System.out.println(line);
                String[] lineInfo = line.split(" ");
                switch (count)
                {
                    case 0:
                        break;
                    case 1:
                        maxY = Integer.parseInt(lineInfo[1]);
                        break;
                    case 2:
                        maxX = Integer.parseInt(lineInfo[1]);
                        break;
                    case 3:
                        tiles = new Tile[maxX][maxY];
                        break;
                }

                count += 1;
            }
            else
            {
                lines.add(sc.nextLine());
            }
        }


        for (int y = 0; y < lines.size(); y++)
        {
            char[] chars = lines.get(y).toCharArray();
            for (int x = 0; x < chars.length; x++)
            {
                char c = chars[x];
                Tile tile = new Tile(x, y, c == '@' ? TileType.BLOCKED : TileType.EMPTY);
                tiles[x][y] = tile;
                allTiles.add(tile);
            }
        }
    }

    private Tile GetTileAt(int x, int y)
    {
        if (!(x > 0 && y > 0 && x < 520 && y < 520)) return null;

        Tile tile = tiles[x][y];
        if (tile == null) return null;

        if (explored.contains(tile) || frontier.contains(tile)) return null;

        return tile.tileType == TileType.BLOCKED ? null : tile;
    }

    private void AddToFrontier(Tile tile)
    {
        if (tile == null) return;

        frontier.add(tile);
        tile.setFill(Color.AQUA);
    }

    private void OnKeyPressed(KeyEvent e)
    {
        System.out.println(frontier.size());

        if (e.getCode() == KeyCode.ENTER)
        {
            UpdateSearch();
        }
        else if (e.getCode() == KeyCode.Q)
        {
            tickSearch = true;
        }
        else if (e.getCode() == KeyCode.P)
        {
            Printer pdfPrinter = null;
            for (Printer printer : Printer.getAllPrinters())
                if (printer.getName().endsWith("PDF"))
                    pdfPrinter = printer;

            assert pdfPrinter != null;
            PrinterJob job = PrinterJob.createPrinterJob(pdfPrinter);

//            centerGroup.setScaleX(0.1);
//            centerGroup.setScaleY(0.1);
//            centerGroup.setScaleZ(0.1);

//                root.setCenter(null);
//                root.setLeft(centerGroup);p

            PageLayout layout = pdfPrinter.createPageLayout(Paper.A3, PageOrientation.LANDSCAPE, Printer.MarginType.HARDWARE_MINIMUM);

            job.getJobSettings().setPageLayout(layout);
            job.getJobSettings().setJobName("Sample Printing Job");

            job.showPrintDialog(stage); // Window must be your main Stage
            job.printPage(centerGroup);


//                root.setLeft(null);
//                root.setCenter(centerGroup);
//                root.setScaleX(1);
//                root.setScaleY(1);
//                root.setScaleZ(1);
            job.endJob();
        }
    }


    private void OnKeyReleased(KeyEvent e)
    {
        if (e.getCode() == KeyCode.Q)
        {
            tickSearch = false;
        }
    }

    private void OnScrolled(ScrollEvent e)
    {
        double scaleDelta = 1 + Math.signum(e.getDeltaY()) * 0.05f;
        centerGroup.setScaleX(centerGroup.getScaleX() * scaleDelta);
        centerGroup.setScaleY(centerGroup.getScaleY() * scaleDelta);
    }

    private void UpdateSearch()
    {
        if (frontier.isEmpty())
        {
            if (goal == null || start == null)
            {
                System.err.println("Need to set a valid start and goal");
                return;
            }

            AddToFrontier(start);
        }

        Tile tile = frontier.poll();
        if (tile == goal)
        {
            frontier.clear();
            explored.clear();
            allTiles.forEach(t -> t.SetType(t.tileType));
            return;
        }

        tile.setFill(Color.DARKBLUE);
        explored.add(tile);

        // Add neighbours to frontier
        AddToFrontier(GetTileAt(tile.tilePos.x - 1, tile.tilePos.y));
        AddToFrontier(GetTileAt(tile.tilePos.x, tile.tilePos.y + 1));
        AddToFrontier(GetTileAt(tile.tilePos.x + 1, tile.tilePos.y));
        AddToFrontier(GetTileAt(tile.tilePos.x, tile.tilePos.y - 1));
    }

    private int CalculateCost(Tile action)
    {
        return Math.abs(start.tilePos.x - action.tilePos.x) + Math.abs(start.tilePos.y - action.tilePos.y);
    }

    private double CalculateHeuristic(Tile action)
    {
        return Math.sqrt(Math.pow(goal.tilePos.x - action.tilePos.x, 2) + Math.pow(goal.tilePos.y - action.tilePos.y, 2));
    }


    private void OnDragged(MouseEvent e)
    {
        if (hasClicked)
        {
            centerGroup.setTranslateX(centerGroup.getTranslateX() + (e.getX() - mouseClicked.x));
            centerGroup.setTranslateY(centerGroup.getTranslateY() + (e.getY() - mouseClicked.y));

            UpdateVisibleTiles();
        }
        else
        {
            mouseClicked.set((float) e.getX(), (float) e.getY());
            hasClicked = true;
        }
    }

    private void UpdateVisibleTiles()
    {
        double xS = -centerGroup.getTranslateX();
        double yS = -centerGroup.getTranslateY();

        double xE = xS + scene.getWidth() * 1.25f;
        double yE = yS + scene.getHeight() * 1.25f;

        centerGroup.getChildren().clear();

        for (int x = (int) (xS / Tile.GRID_SIZE); x < (int) (xE / Tile.GRID_SIZE); x++)
        {
            for (int y = (int) (yS / Tile.GRID_SIZE); y < (int) (yE / Tile.GRID_SIZE); y++)
            {
                if (x < 0 || y < 0 || x > 512 || y > 512) continue;

                if (tiles[x][y] != null)
                    centerGroup.getChildren().add(tiles[x][y]);
            }
        }
    }

    public static void main(String[] args)
    {
        launch(args);
    }
}
