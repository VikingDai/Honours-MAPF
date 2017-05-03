package search;

import java.util.Comparator;

public class SearchNodeComparator implements Comparator<SearchNode>
{
    @Override
    public int compare(SearchNode n1, SearchNode n2)
    {
        return Integer.signum(n1.f - n2.f);
    }
}
