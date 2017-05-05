package search;


import domains.GridMapMetaInfo;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Scanner;

public class ProblemInstance
{
    private int goal;
    private int start;
    private int searchId;

    public String mapFileName;

    public ProblemInstance(String fileName)
    {
        goal = 0;
        start = 0;

        loadFile(fileName);
    }

    public boolean loadFile(String fileName)
    {
        Scanner scanner;
        try
        {
            scanner = new Scanner(new File(fileName));
        } catch (FileNotFoundException e)
        {
            e.printStackTrace();
            return false;
        }
        assert scanner.hasNextLine();
        mapFileName = scanner.nextLine();
        return true;
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
