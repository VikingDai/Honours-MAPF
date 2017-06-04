package utils;

import javafx.scene.canvas.GraphicsContext;
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

    public boolean isOccupied(int timestep, int x, int y)
    {
        if (!table.containsKey(timestep))
            return false;
        Point pos = new Point(x, y);

        if (!table.get(timestep).containsKey(pos)) return false;

        return table.get(timestep).get(pos).size() > 0;
    }

    public void addPath(Agent agent, Stack<SearchNode> path)
    {
        int count = 0;
        SearchNode current = path.pop();//agent.currentNode;
        if (path.empty()) return;

        while (!path.empty())
        {
            SearchNode next = path.pop();
            int dx = next.x == current.x ? 0 : (int) Math.signum(next.x - current.x);
            int dy = next.y == current.y ? 0 : (int) Math.signum(next.y - current.y);

            int cx = current.x;
            int cy = current.y;
            while (next.x != cx || next.y != cy)
            {
                System.out.println(cx + " " + cy + " ");
                count += 1;
                cx += dx;
                cy += dy;

                int timestep = Main.getSimulation().timeStep + count;
                Point currPoint = new Point(cx, cy);

                if (!table.containsKey(timestep))
                    table.put(timestep, new HashMap<>());

                if (!table.get(timestep).containsKey(currPoint))
                    table.get(timestep).put(currPoint, new ArrayList<>());

                table.get(timestep).get(currPoint).add(agent);
            }
            current = next;
        }
    }

    public ReservationTable()
    {
        collisions = new HashMap<>();
        table = new HashMap<>();
    }

    public void update(int timestep)
    {
        if (table.containsKey(timestep))
        {
            table.remove(timestep);
            System.out.println("Removed " + timestep);
        }
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
//            assert time > timeStep : "Error why dis happen";

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

    public void draw(GraphicsContext gc)
    {
        for (Map<Point, List<Agent>> pointListMap : table.values())
        {
            for (Point point : pointListMap.keySet())
            {
                boolean collision = pointListMap.get(point).size() > 1;
                gc.setFill(collision ? Color.RED : Color.BLACK);
                gc.fillOval(
                        point.x * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2 - Globals.RENDER_SCALE * 0.25,
                        point.y * Globals.RENDER_SCALE + Globals.RENDER_SCALE / 2 - Globals.RENDER_SCALE * 0.25,
                        Globals.RENDER_SCALE * 0.5,
                        Globals.RENDER_SCALE * 0.5);
            }
        }
    }
}
