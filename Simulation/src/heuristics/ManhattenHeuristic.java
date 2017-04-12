package heuristics;

public class ManhattenHeuristic extends BaseHeuristic
{
    public ManhattenHeuristic()
    {

    }

    @Override
    public int h(int x1, int y1, int x2, int y2)
    {
        return (Math.abs(x2 - x1) + Math.abs(y2 - y1));
    }
}
