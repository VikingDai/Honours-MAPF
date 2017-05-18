package utils;

import search.SearchNode;

import java.util.List;

public interface ISearch
{
    List<SearchNode> findPath(SearchNode start, SearchNode goal);
}