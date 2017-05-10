package sample;

import javafx.scene.input.MouseEvent;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import search.SearchNode;
import utils.Globals;

import java.awt.*;

public class Tile extends Rectangle
{
    public enum TileType
    {
        EMPTY, BLOCKED, START, GOAL
    }

    //    public static double GRID_SIZE = 40;
    public Point tilePos;
    public TileType tileType;
    public SearchNode node;

    public Tile(int x, int y, TileType tileType)
    {

        super(  x * (Globals.RENDER_SCALE + 1),
                y * (Globals.RENDER_SCALE + 1),
                Globals.RENDER_SCALE,
                Globals.RENDER_SCALE);

        SetType(tileType);
        tilePos = new Point(x, y);
        setOnMouseClicked(this::OnMouseClicked);
    }

    public void setSearchNode(SearchNode node)
    {
        this.node = node;
    }

    public void SetType(TileType newType)
    {
        switch (newType)
        {
            case EMPTY:
                setFill(Color.WHITE);
                break;
            case BLOCKED:
                setFill(new Color(0.2, 0.2, 0.2, 0.5));
                break;
            case START:
                setFill(Color.GREEN);
                break;
            case GOAL:
                setFill(Color.RED);
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
                    case PRIMARY:
                        if (node != null) SetType(TileType.START);

                        break;
                    case SECONDARY:
                        if (node != null) SetType(TileType.GOAL);
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
