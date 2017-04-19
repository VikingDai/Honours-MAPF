package sample;

import javafx.scene.input.MouseEvent;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Text;

import java.awt.*;

public class Tile extends Rectangle
{

    public enum TileType
    {
        EMPTY, BLOCKED, START, GOAL
    }

    public static float GRID_SIZE = 8;
    public Point tilePos;
    public TileType tileType;

    public Tile(int x, int y, TileType tileType)
    {
        super(x, y, GRID_SIZE - 0, GRID_SIZE - 0);

        SetType(tileType);
        tilePos = new Point(x, y);

        setStroke(javafx.scene.paint.Color.BLACK);
        setStrokeWidth(.5f);
        setX(x * GRID_SIZE);
        setY(y * GRID_SIZE);

        setOnMouseClicked(this::OnMouseClicked);

    }

    public void SetType(TileType newType)
    {
//        if (tileType == TileType.START)
//            Main.start = null;
//        if (tileType == TileType.GOAL)
//            Main.goal = null;

        switch (newType)
        {
            case EMPTY:
                setFill(Color.WHITE);
                break;
            case BLOCKED:
                setFill(Color.GREY);
                break;
            case START:
                setFill(Color.GREEN);
//                Main.start = this;
                break;
            case GOAL:
                setFill(Color.RED);
//                Main.goal = this;
                break;
        }

        tileType = newType;
    }

    private void OnMouseClicked(MouseEvent e)
    {
        switch (tileType)
        {
            case EMPTY:
                switch (e.getButton())
                {
//                    case PRIMARY:
//                        if (Main.start != null) Main.start.SetType(TileType.EMPTY);
//                        SetType(TileType.START);
//                        break;
//                    case SECONDARY:
//                        if (Main.goal != null) Main.goal.SetType(TileType.EMPTY);
//                        SetType(TileType.GOAL);
                }
                break;
            case START:
                SetType(TileType.EMPTY);
                break;
            case GOAL:
                SetType(TileType.EMPTY);
                break;
            case BLOCKED:
                break;
        }
    }
}
