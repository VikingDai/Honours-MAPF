import com.sun.javafx.geom.Vec2f;
import domains.GridMap;
import expanders.GridMapExpansionPolicy;
import heuristics.ManhattenHeuristic;
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
import sample.Tile;
import search.FlexibleAStar;
import domains.GridMapParser;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.*;

import static java.lang.Thread.sleep;


public class Main extends Application
{
    private Tile[][] tiles;

    private Vec2f mouseClicked = new Vec2f();
    private boolean hasClicked = false;

    private Stage stage;
    private BorderPane root;
    private Group centerGroup;
    private Scene scene;

    private boolean tickSearch = false;
    private boolean searchOver = false;

    private Simulation simulation;
    private GridMap map;
    FlexibleAStar aStar;

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
//        LoadMap("maps/newMap.map");
//        LoadMap("maps/AcrosstheCape.map");

        // 144 fps
        Timeline timeline = new Timeline(new KeyFrame(Duration.millis(7), this::OnTick));
        timeline.setCycleCount(Animation.INDEFINITE);
        timeline.play();
        UpdateVisibleTiles();


        // Testing
        simulation = new Simulation();

        GridMapParser mapParser = new GridMapParser("maps/newMap.map");
//        mapParser.loadFile("maps/AcrosstheCape.map");
        map = new GridMap(mapParser);

        aStar = new FlexibleAStar<>(new ManhattenHeuristic(mapParser.getMetaInfo()), new GridMapExpansionPolicy(map));
        map.getTiles().forEach((Tile t) -> centerGroup.getChildren().add(t));

        aStar.search(1, 10, 10, 1);

//        tiles = new Tile[mapParser.getMetaInfo().getWidth()][mapParser.getMetaInfo().getHeight()];
//        allTiles = mapParser.createTiles(tiles);
    }

    private void OnTick(ActionEvent e)
    {
//        simulation.update(7);
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
//                        tiles = new Tile[maxX][maxY];
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
//                Tile tile = new Tile(x, y, c == '@' ? Tile.TileType.BLOCKED : Tile.TileType.EMPTY);
//                tiles[x][y] = tile;
//                allTiles.add(tile);
            }
        }
    }

    private void OnKeyPressed(KeyEvent e)
    {
        if (e.getCode() == KeyCode.ENTER)
        {
            aStar.step();
//            UpdateSearch();
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

            PageLayout layout = pdfPrinter.createPageLayout(Paper.A3, PageOrientation.LANDSCAPE, Printer.MarginType.HARDWARE_MINIMUM);

            job.getJobSettings().setPageLayout(layout);
            job.getJobSettings().setJobName("Sample Printing Job");

            job.showPrintDialog(stage); // Window must be your main Stage
            job.printPage(centerGroup);

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
//        double xS = -centerGroup.getTranslateX();
//        double yS = -centerGroup.getTranslateY();
//
//        double xE = xS + scene.getWidth() * 1.25f;
//        double yE = yS + scene.getHeight() * 1.25f;
//
//        centerGroup.getChildren().clear();
//
//        for (int x = (int) (xS / Tile.GRID_SIZE); x < (int) (xE / Tile.GRID_SIZE); x++)
//        {
//            for (int y = (int) (yS / Tile.GRID_SIZE); y < (int) (yE / Tile.GRID_SIZE); y++)
//            {
//                if (x < 0 || y < 0 || x > 512 || y > 512) continue;
//
//                if (tiles[x][y] != null)
//                    centerGroup.getChildren().add(tiles[x][y]);
//            }
//        }
    }

    public static void main(String[] args)
    {
        launch(args);
    }
}
