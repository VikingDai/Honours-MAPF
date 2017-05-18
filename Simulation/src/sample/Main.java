package sample;

import graphics.GUI;
import graphics.GUI.RenderLayer;
import javafx.animation.Animation;
import javafx.animation.KeyFrame;
import javafx.animation.Timeline;
import javafx.application.Application;
import javafx.event.ActionEvent;
import javafx.print.*;
import javafx.scene.Scene;
import javafx.scene.input.KeyEvent;
import javafx.stage.Stage;
import javafx.util.Duration;


public class Main extends Application
{
    private static Stage stage;
    public static Stage getStage() { return stage; }

    private static Scene scene;
    public static Scene getScene() { return scene; }

    private static Simulation simulation;
    public static Simulation getSimulation()
    {
        return simulation;
    }

    private static GUI gui;
    public static GUI getGUI()
    {
        return gui;
    }

    @Override
    public void start(Stage primaryStage) throws Exception
    {
//        Parent simulationGroup = FXMLLoader.load(getClass().getResource("sample.fxml"));
        double width = 1280;
        double height = 720;

        // 144 fps
        Timeline timeline = new Timeline(new KeyFrame(Duration.millis(7), this::OnTick));
        timeline.setCycleCount(Animation.INDEFINITE);
        timeline.play();

        // make a new simulation
        simulation = new Simulation();

        gui = new GUI();

        // Scene is the window
        scene = new Scene(gui, width, height);
        stage = primaryStage;
        stage.setTitle("Warehouse Automation");
        stage.setScene(scene);
        stage.sizeToScene();
        stage.show();

        scene.setOnKeyPressed(this::OnKeyPressed);

//        simulation.getMap().getTiles().forEach(t -> simulationGroup.getChildren().add(t));
//        UpdateVisibleTiles();
//        simulation.getAgents().forEach(a -> simulationGroup.getChildren().add(a));

        simulation.init("maps/warehouse.map");
        simulation.agents.forEach(a -> gui.simulationGroup.getChildren().add(a.circle));
        simulation.drawMap(gui.getLayer(RenderLayer.BACKGROUND));
    }

    private void OnTick(ActionEvent e)
    {

//            simulation.step(gui.getLayer(RenderLayer.MOVING));
//            gui.setTimeStep(simulation.timeStep);
        simulation.tick(0.016f);
    }

    private void OnKeyPressed(KeyEvent e)
    {
        switch (e.getCode())
        {
            case ENTER:
                simulation.step(gui.getLayer(RenderLayer.MOVING));
                gui.setTimeStep(simulation.timeStep);
                break;
            case P:
                printCenterStage();
        }
    }

    private void printCenterStage()
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
//        job.printPage(simulationGroup);

        job.endJob();
    }

    public static void main(String[] args)
    {
        launch(args);
    }




}
