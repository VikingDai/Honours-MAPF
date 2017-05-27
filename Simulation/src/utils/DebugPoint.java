package utils;

import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import search.SearchNode;

public class DebugPoint
{
    int x;
    int y;
    Color color;
    double size;

    public DebugPoint(int x, int y, Color color, double size)
    {
        this.x = x;
        this.y = y;
        this.color = color;
        this.size = size;
    }

    public DebugPoint(SearchNode node, Color color, double size)
    {
        this(node.x, node.y, color, size);
    }

    public void drawPoint(GraphicsContext gc)
    {
        gc.setFill(color);
        gc.fillOval(
                x * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2 - Globals.RENDER_SCALE * size / 2,
                y * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2 - Globals.RENDER_SCALE * size / 2,
                Globals.RENDER_SCALE * size,
                Globals.RENDER_SCALE * size);
    }
}
