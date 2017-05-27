package utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class Globals
{
    public static double RENDER_SCALE = 40;
    public static int SEED = 0;
    public static Random RNG = new Random(SEED);
    public static List<DebugPoint> debugPoints = new ArrayList<>();
}
