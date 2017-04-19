package heuristics;

public abstract class BaseHeuristic
{
    public abstract int h(int startId, int goalId);
    public abstract int h(int startX, int startY, int goalX, int goalY);
}
