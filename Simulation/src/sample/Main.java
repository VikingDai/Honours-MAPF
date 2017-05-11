package sample;

import com.sun.javafx.geom.Vec2d;
import javafx.animation.Animation;
import javafx.animation.KeyFrame;
import javafx.animation.Timeline;
import javafx.application.Application;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.geometry.Insets;
import javafx.print.*;
import javafx.scene.Group;
import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.*;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.ScrollEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Pane;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.util.Duration;
import javafx.util.converter.IntegerStringConverter;
import utils.Globals;

import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;


public class Main extends Application
{
    public static int CANVAS_MAX_SIZE = 5000;

    private Vec2d mouseClicked = new Vec2d();
    private boolean hasClicked = false;

    private Stage stage;
    private Group centerGroup;
    private Scene scene;
    private GraphicsContext tileGC;
    private GraphicsContext agentGC;
    private GraphicsContext pathGC;

    private boolean updateTiles = true;

    private static Simulation simulation;

    private Label timeStepLabel;

    @Override
    public void start(Stage stage) throws Exception
    {
//        Parent centerGroup = FXMLLoader.load(getClass().getResource("sample.fxml"));
        double width = 1280;
        double height = 720;

        Canvas tileCanvas = new Canvas(CANVAS_MAX_SIZE, CANVAS_MAX_SIZE);
        tileGC = tileCanvas.getGraphicsContext2D();

        Canvas agentCanvas = new Canvas(CANVAS_MAX_SIZE, CANVAS_MAX_SIZE);
        agentGC = agentCanvas.getGraphicsContext2D();

        Canvas pathCanvas = new Canvas(CANVAS_MAX_SIZE, CANVAS_MAX_SIZE);
        pathGC = agentCanvas.getGraphicsContext2D();

        // sample.Main homeNode where rendering occurs
        centerGroup = new Group();
        centerGroup.setOnMouseDragged(this::OnDragged);
        centerGroup.setOnMouseReleased(e -> hasClicked = false);

        Pane layeredPane = new Pane();
        Pane agentPane = new Pane();

        layeredPane.getChildren().add(agentPane);
        layeredPane.getChildren().add(tileCanvas);
        layeredPane.getChildren().add(agentCanvas);
        layeredPane.getChildren().add(pathCanvas);
        tileCanvas.toFront();
        agentPane.toFront();
        agentCanvas.toFront();
        pathCanvas.toFront();

//        layeredPane.setOnMouseDragged(this::OnDragged);
//        layeredPane.setOnMouseReleased(e -> hasClicked = false);

        centerGroup.getChildren().add(layeredPane);
        BorderPane borderPane = new BorderPane(centerGroup);
        borderPane.setStyle("-fx-background-color: #000000;");


        // Top group
        Group topGroup = new Group();
//        borderPane.setTop(topGroup);

        HBox topBox = new HBox();

        topBox.setPrefHeight(30);
        topBox.setPadding(new Insets(10, 10, 10, 10));
        topBox.setSpacing(20);
        topBox.setStyle("-fx-background-color: #FFFFFF; -fx-border-insets: 0 0 0 0; -fx-border-color: #D3D3D3");
//        topBox.setAlignment(Pos.BASELINE_CENTER);
//        topBox.setBorder(new Border());
        topGroup.getChildren().add(topBox);

        borderPane.setTop(topBox);

        // radio button
        RadioButton radioButton = new RadioButton("test");

        // average search time label
        Label avgTime = new Label("Average search time: 0.001 ms");

        // time step label
        timeStepLabel = new Label("Time step: 0");

        // all maps combobox
        ObservableList<Path> allMaps = FXCollections.observableArrayList();
        Files.list(Paths.get("maps")).forEach(allMaps::add);
        ComboBox<Path> mapComboBox = new ComboBox<>(allMaps);

        Button loadMapButton = new Button("Load Map");
        loadMapButton.setOnAction(event ->
        {
            if (mapComboBox.getValue() != null)
            {
                System.out.println("Loaded map: " + mapComboBox.getValue());
                simulation.init(mapComboBox.getValue().toString());
            }
            else
            {
                System.err.println("Map selected is null");
            }
        });


        // choose file button
        Button chooseFileButton = new Button("Open Map");
        chooseFileButton.setOnAction(event ->
        {
            FileChooser fileChooser = new FileChooser();
            fileChooser.setTitle("Open Resource File");
            fileChooser.showOpenDialog(stage);
        });

        // enter seed
        TextField seedTextField = new TextField();
        seedTextField.setTextFormatter(new TextFormatter<>(new IntegerStringConverter()));
        seedTextField.setText("" + 0);


        // run button
        ImageView runImage = new ImageView(new Image("file:assets/images/play-button.png"));
        runImage.setPreserveRatio(true);
        runImage.setFitHeight(topBox.getPrefHeight());
        Button runButton = new Button("", runImage);
        runButton.setOnAction(event ->
        {
            simulation.step(pathGC);
//            agentGC.clearRect(0, 0, CANVAS_MAX_SIZE, CANVAS_MAX_SIZE);
//            simulation.draw(pathGC);
//            System.out.println("Running simulation");
        });

        // pause button
        ImageView pauseImage = new ImageView(new Image("file:assets/images/pause-button.png"));
        pauseImage.setPreserveRatio(true);
        pauseImage.setFitHeight(topBox.getPrefHeight());
        Button pauseButton = new Button("", pauseImage);
        pauseButton.setOnAction(event -> {});


        topBox.getChildren().addAll(mapComboBox, loadMapButton, chooseFileButton, radioButton, avgTime, seedTextField, runButton, pauseButton, timeStepLabel);

        // Scene is the window
        scene = new Scene(borderPane, width, height);
        this.stage = stage;
        stage.setTitle("Warehouse Automation");
        stage.setScene(scene);
        stage.sizeToScene();
        stage.show();

        scene.setOnKeyPressed(this::OnKeyPressed);
        scene.setOnKeyReleased(this::OnKeyReleased);
        scene.setOnScroll(this::OnScrolled);

        // 144 fps
        Timeline timeline = new Timeline(new KeyFrame(Duration.millis(7), this::OnTick));
        timeline.setCycleCount(Animation.INDEFINITE);
        timeline.play();

        // run the simulation
        simulation = new Simulation();
        simulation.init("maps/warehouse.map");
        simulation.agents.forEach(a -> agentPane.getChildren().add(a.circle));


//        simulation.getMap().getTiles().forEach(t -> centerGroup.getChildren().add(t));
//        UpdateVisibleTiles();
//        simulation.getAgents().forEach(a -> centerGroup.getChildren().add(a));

        simulation.drawMap(tileGC);
    }

    private void OnTick(ActionEvent e)
    {
        simulation.tick(0.016f);
    }

    private void OnKeyPressed(KeyEvent e)
    {
        if (e.getCode() == KeyCode.ENTER) // step simulation
        {
//            agentGC.clearRect(0, 0, CANVAS_MAX_SIZE, CANVAS_MAX_SIZE);
            simulation.step(pathGC);
            timeStepLabel.setText(simulation.timeStep + "");


        }
        else if (e.getCode() == KeyCode.P)
        {
            PrintCenterStage();
        }
    }

    private void PrintCenterStage()
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


    private void OnKeyReleased(KeyEvent e)
    {
        if (e.getCode() == KeyCode.Q)
        {
//            tickSearch = false;
        }
    }

    // ZOOM
    private void OnScrolled(ScrollEvent e)
    {
        // decrease scale
        double scaleDelta = 1 + Math.signum(e.getDeltaY()) * 0.05f;
        Globals.RENDER_SCALE *= scaleDelta;

        // redraw graphics
        simulation.drawMap(tileGC);
        simulation.draw(pathGC);
    }

    private void OnDragged(MouseEvent e)
    {
        if (hasClicked)
        {
            centerGroup.setTranslateX(centerGroup.getTranslateX() + (e.getX() - mouseClicked.x));
            centerGroup.setTranslateY(centerGroup.getTranslateY() + (e.getY() - mouseClicked.y));
        }
        else
        {
            mouseClicked.set(e.getX(), e.getY());
            hasClicked = true;
        }
    }

    public static Simulation getSimulation()
    {
        return simulation;
    }

    public static void main(String[] args)
    {
        launch(args);
    }
}
