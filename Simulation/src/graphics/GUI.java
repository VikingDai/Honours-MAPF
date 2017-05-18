package graphics;

import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.geometry.Insets;
import javafx.scene.Group;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.*;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.ScrollEvent;
import javafx.scene.layout.*;
import javafx.stage.FileChooser;
import javafx.util.converter.IntegerStringConverter;
import math.Vector2d;
import sample.Main;
import sample.Simulation;
import utils.Globals;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;

public class GUI extends BorderPane
{
    public enum RenderLayer
    {
        BACKGROUND,
        MOVING,
        AGENTS,
    }


    // canvas parameters
    public static int CANVAS_MAX_SIZE = 5000;
    public Group simulationGroup;
    private Map<RenderLayer, GraphicsContext> renderLayers;

    //
    private boolean hasClicked;
    private Vector2d mouseClicked;

    private Label timeStepLabel;

    public GUI()
    {
        // setup input parameters
        hasClicked = false;
        mouseClicked = new Vector2d(0, 0);

        setStyle("-fx-background-color: #000000;");

        // create render layers and add to hash map
        renderLayers = new HashMap<>();
        Canvas tileCanvas = new Canvas(CANVAS_MAX_SIZE, CANVAS_MAX_SIZE);
        renderLayers.put(RenderLayer.BACKGROUND, tileCanvas.getGraphicsContext2D());

        Canvas agentCanvas = new Canvas(CANVAS_MAX_SIZE, CANVAS_MAX_SIZE);
        renderLayers.put(RenderLayer.AGENTS, agentCanvas.getGraphicsContext2D());

        Canvas movingCanvas = new Canvas(CANVAS_MAX_SIZE, CANVAS_MAX_SIZE);
        renderLayers.put(RenderLayer.MOVING, movingCanvas.getGraphicsContext2D());


        // sample.Main homeNode where rendering occurs
        simulationGroup = new Group();
        simulationGroup.setOnMouseDragged(this::onDragged);
        simulationGroup.setOnScroll(this::onScrolled);
        simulationGroup.setOnMouseReleased(e -> hasClicked = false);

        setCenter(simulationGroup);

        Pane layeredPane = new Pane();
        Pane agentPane = new Pane();
        layeredPane.getChildren().add(agentPane);
        layeredPane.getChildren().add(tileCanvas);
        layeredPane.getChildren().add(agentCanvas);
        layeredPane.getChildren().add(movingCanvas);

        tileCanvas.toFront();
        agentPane.toFront();
        movingCanvas.toFront();
        agentCanvas.toFront();

        simulationGroup.getChildren().add(layeredPane);

        setupTopPanel();

        try
        {
            setupSidePanel();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    private void setupTopPanel()
    {
        HBox box = new HBox();
        setTop(box);
        box.setPrefHeight(30);
        box.setPadding(new Insets(10, 10, 10, 10));
        box.setSpacing(20);
        box.setStyle("-fx-background-color: #FFFFFF; -fx-border-insets: 0 0 0 0; -fx-border-color: #D3D3D3");

        timeStepLabel = new Label("Time step: 0"); // timestep

        Label avgTime = new Label("Average search time: 0.001 ms"); // search time

        // enter seed
        TextField seedTextField = new TextField();
        seedTextField.setTextFormatter(new TextFormatter<>(new IntegerStringConverter()));
        seedTextField.setText("" + 0);

        Label seedLabel = new Label("Seed: 0");

        box.getChildren().addAll(timeStepLabel, avgTime, seedLabel);
    }

    private void setupSidePanel() throws IOException
    {
        VBox box = new VBox();
        box.setPrefHeight(30);
        box.setPadding(new Insets(10, 10, 10, 10));
        box.setSpacing(20);
        box.setStyle("-fx-background-color: #FFFFFF; -fx-border-insets: 0 0 0 0; -fx-border-color: #D3D3D3");
        setLeft(box);

        RadioButton radioButton = new RadioButton("test"); // testing radio button

        // all maps combo box
        ObservableList<Path> allMaps = FXCollections.observableArrayList();
        Files.list(Paths.get("maps")).forEach(allMaps::add);
        ComboBox<Path> mapComboBox = new ComboBox<>(allMaps);

        Button loadMapButton = new Button("Load Map");
        loadMapButton.setOnAction(event ->
        {
            if (mapComboBox.getValue() != null)
            {
                System.out.println("Loaded map: " + mapComboBox.getValue());
                Main.getSimulation().init(mapComboBox.getValue().toString());
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
//            fileChooser.showOpenDialog();
        });


        // run button
        ImageView runImage = new ImageView(new Image("file:assets/images/play-button.png"));
        runImage.setPreserveRatio(true);
        runImage.setFitHeight(box.getPrefHeight());
        Button runButton = new Button("", runImage);
        runButton.setOnAction(event -> Main.getSimulation().isTicking = true);

        // pause button
        ImageView pauseImage = new ImageView(new Image("file:assets/images/pause-button.png"));
        pauseImage.setPreserveRatio(true);
        pauseImage.setFitHeight(box.getPrefHeight());
        Button pauseButton = new Button("", pauseImage);
        pauseButton.setOnAction(event -> Main.getSimulation().isTicking = false);

        box.getChildren().addAll(mapComboBox, loadMapButton, chooseFileButton, radioButton, runButton, pauseButton);
    }

    private void onScrolled(ScrollEvent e)
    {
        // decrease scale
        double scaleDelta = 1 + Math.signum(e.getDeltaY()) * 0.05f;
        Globals.RENDER_SCALE *= scaleDelta;

        // redraw graphics
        Main.getSimulation().drawMap(getLayer(RenderLayer.BACKGROUND));
    }

    private void onDragged(MouseEvent e)
    {
        if (hasClicked)
        {
            getCenter().setTranslateX(getCenter().getTranslateX() + (e.getX() - mouseClicked.x));
            getCenter().setTranslateY(getCenter().getTranslateY() + (e.getY() - mouseClicked.y));
        }
        else
        {
            mouseClicked.set(e.getX(), e.getY());
            hasClicked = true;
        }
    }

    public GraphicsContext getLayer(RenderLayer layer) { return renderLayers.get(layer); }

    public void setTimeStep(int timeStep) { timeStepLabel.setText("Time step: " + timeStep); }
}
