package utils;

import javafx.scene.paint.Color;
import sample.Agent;
import sample.Main;
import search.SearchNode;

import java.awt.*;
import java.util.*;
import java.util.List;

public class ReservationTable
{
    public HashMap<Integer, List<Point>> collisions;

    // int => (x, y) => List<Agent>
    public Map<Integer, Map<Point, List<Agent>>> table;

    public ReservationTable()
    {
        collisions = new HashMap<>();
        table = new HashMap<>();
    }

    public void update(int timestep, List<Agent> agents)
    {
        agents.forEach(a -> update(timestep, a));
    }

    public void update(int timestep, Agent agent)
    {
        if (table.containsKey(timestep))
        {
            table.remove(timestep);
            collisions.remove(timestep);
            System.out.println("Removed " + timestep);
        }

        ArrayList<SearchNode> path = new ArrayList<>(agent.path);
        for (int i = 0; i < path.size(); i++)
        {
            int timeAtPosition = i + 1 + timestep;

            // point in path
            SearchNode searchNode = path.get(i);
            Point agentPos = new Point(searchNode.x, searchNode.y);

            if (!table.containsKey(timeAtPosition))
                table.put(timeAtPosition, new HashMap<>());

            Map<Point, List<Agent>> positionAgentMap = table.get(timeAtPosition);
            if (!positionAgentMap.containsKey(agentPos))
                positionAgentMap.put(agentPos, new ArrayList<>());

            List<Agent> agentsAtPositionTime = positionAgentMap.get(agentPos);
            agentsAtPositionTime.add(agent);

        }
    }

    public void findConflicts(int timestep)
    {
        for (Integer time : table.keySet())
        {
//            assert time > timestep : "Error why dis happen";

            Map<Point, List<Agent>> positionToAgents = table.get(time);

            for (Point position : positionToAgents.keySet())
            {
                List<Agent> agents = positionToAgents.get(position);
                if (agents.size() > 1)
                {
                    System.out.println("Collision at: {" + position.x + ", " + position.y + "} Time " + time + " CURRENTLY " + timestep);
                    if (!collisions.containsKey(time))
                        collisions.put(time, new ArrayList<>());

                    collisions.get(time).add(position);
                }


//                System.out.println("\t" + agents.size() + " agents at {" + position.x + ", " + position.y + "}, time " + t);
            }
        }
    }

    public void print()
    {
        for (int t = 0; t < table.size(); t++)
        {
            System.out.println("Time: " + t);
            Map<Point, List<Agent>> positionToAgents = table.get(t);

            for (Point position : positionToAgents.keySet())
            {
                List<Agent> agents = table.get(t).get(position);
                System.out.println("\t" + agents.size() + "agents at {" + position.x + ", " + position.y + "}");
            }
        }
    }
}
