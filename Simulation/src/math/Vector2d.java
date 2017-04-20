package math;

import com.sun.javafx.geom.Vec2d;

public class Vector2d extends Vec2d
{
    public Vector2d(double x, double y)
    {
        super(x, y);
    }

    public Vector2d add(Vector2d other)
    {
        x += other.x;
        y += other.y;
        return this;
    }

    public Vector2d sub(Vector2d other)
    {
        x -= other.x;
        y -= other.y;
        return this;
    }

    public Vector2d mul(double value)
    {
        x *= value;
        y *= value;
        return this;
    }

    public Vector2d div(double value)
    {
        if (value == 0) return this;
        x /= value;
        y /= value;
        return this;
    }

    public double length()
    {
        return Math.sqrt(x * x + y * y);
    }

    public Vector2d normalize()
    {
        return div(length());
    }

    public Vector2d copy()
    {
        return new Vector2d(x, y);
    }
}
