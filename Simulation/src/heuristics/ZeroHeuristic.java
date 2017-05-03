package heuristics;

public class ZeroHeuristic extends BaseHeuristic
{
    public ZeroHeuristic() { }

    @Override
    public int h(int startId, int goalId)
    {
        return 0;
    }

    @Override
    public int h(int x1, int y1, int x2, int y2)
    {
        return 0;
    }
}
