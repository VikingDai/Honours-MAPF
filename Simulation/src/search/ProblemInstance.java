package search;


public class ProblemInstance
{
    private int goal;
    private int start;
    private int searchId;

    public ProblemInstance(int goalId, int startId)
    {
        goal = 0;
        start = 0;
    }

    public int getGoal()
    {
        return goal;
    }

    public int getSearchId()
    {
        return searchId;
    }

    public void setGoal(int goal)
    {
        this.goal = goal;
    }

    public int getStart()
    {
        return start;
    }

    public void setStart(int start)
    {
        this.start = start;
    }

    public void setSearchId(int searchId)
    {
        this.searchId = searchId;
    }
}
