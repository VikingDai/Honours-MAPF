package sample;


import com.sun.javafx.geom.Vec2d;
import com.sun.javafx.geom.Vec2f;
import expanders.GridMapExpansionPolicy;
import heuristics.ManhattanHeuristic;
import javafx.animation.PathTransition;
import javafx.geometry.Point2D;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.shape.MoveTo;
import javafx.scene.shape.Path;
import javafx.util.Duration;
import math.Vector2d;
import search.FlexibleAStar;
import search.SearchNode;

import java.util.Stack;

public class Agent extends Circle
{
    public Stack<SearchNode> path;
    public SearchNode nextNode;
    public SearchNode currentNode;
    boolean reachedNext;

    private FlexibleAStar<ManhattanHeuristic, GridMapExpansionPolicy> search;

    public Agent(SearchNode currentNode, FlexibleAStar search)
    {
        super(currentNode.getTile().centerX, currentNode.getTile().centerY, Tile.GRID_SIZE);


        setStroke(Color.BLACK);
        setStrokeWidth(1f);

        this.currentNode = currentNode;
        reachedNext = true;
        this.search = search;
        path = new Stack<>();
        setFill(Color.color(Math.random(), Math.random(), Math.random()));
    }

    public void update(float dt)
    {
//        if (nextNode == null) // get new point on path
//        {
//            if (!path.isEmpty())
//            {
//                nextNode = path.pop();
//                Tile nextTile = nextNode.getTile();
//
////                Path path = new Path();
////                path.getElements().add(new MoveTo(nextTile.centerX, nextTile.centerY));
////                PathTransition transition = new PathTransition();
////
////                transition.setDuration(Duration.millis(1000));
////                transition.setNode(this);
////                transition.setPath(path);
////                transition.play();
//            }
//            else
//            {
//                SearchNode randomNode = Main.getSimulation().getMap().getRandomNode();
////                Stack t = ;
//                path.addAll(search.findPath(currentNode.getX(), currentNode.getY(), randomNode.getX(), randomNode.getY()));
//            }
//        }
//        else // follow path
//        {
////
//            Tile nextTile = nextNode.getTile();
//
//            Vector2d nextPos = new Vector2d(nextTile.getX(), nextTile.getY());
//            Vector2d currentPos = new Vector2d(getCenterX(), getCenterY());
//
//            if (nextPos.distanceSq(currentPos) < 0.5f)
//            {
//                currentNode = nextNode;
//                nextNode = null;
//            }
//
//            Vector2d deltaPos = nextPos.sub(currentPos).normalize();
////            System.out.println(deltaPos.x + " " + normPos.x);
//
//            setCenterX(getCenterX() + deltaPos.x * dt);
//            setCenterY(getCenterY() + deltaPos.y * dt);
//        }
    }

    public void step()
    {
        if (path.isEmpty()) // search to random position
        {
            SearchNode randomNode = Main.getSimulation().getMap().getRandomNode();
            path.addAll(search.findPath(currentNode.getX(), currentNode.getY(), randomNode.getX(), randomNode.getY()));
        }

        nextNode = path.pop();
        currentNode = nextNode;
    }
}
