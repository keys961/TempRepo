package com.company;

import java.util.*;
import java.util.List;
import java.util.stream.IntStream;

public class Main
{
    private static HashMap<Integer, List<Integer>> succMap = new HashMap<>();

    private static HashMap<Integer, List<Character>> useMap = new HashMap<>();

    private static HashMap<Integer, List<Character>> defMap = new HashMap<>();

    private static HashMap<Integer, TreeSet<Character>> inMap = new HashMap<>();

    private static HashMap<Integer, TreeSet<Character>> outMap = new HashMap<>();

    private static HashMap<Integer, TreeSet<Character>> oldInMap = new HashMap<>();

    private static HashMap<Integer, TreeSet<Character>> oldOutMap = new HashMap<>();

    static
    {
        IntStream.range(1, 16).forEach(i ->
        {
            inMap.put(i, new TreeSet<>());
            outMap.put(i, new TreeSet<>());
            oldInMap.put(i, null);
            oldOutMap.put(i, null);
        });

        succMap.put(1, List.of(2)); useMap.put(1, List.of()); defMap.put(1, List.of('m'));
        succMap.put(2, List.of(3)); useMap.put(2, List.of()); defMap.put(2, List.of('v'));
        succMap.put(3, List.of(4, 15)); useMap.put(3, List.of('v', 'n')); defMap.put(3, List.of());
        succMap.put(4, List.of(5)); useMap.put(4, List.of('v')); defMap.put(4, List.of('r'));
        succMap.put(5, List.of(6)); useMap.put(5, List.of()); defMap.put(5, List.of('s'));
        succMap.put(6, List.of(7, 9)); useMap.put(6, List.of('n', 'r')); defMap.put(6, List.of());
        succMap.put(7, List.of(8)); useMap.put(7, List.of('v')); defMap.put(7, List.of('v'));
        succMap.put(8, List.of(3)); useMap.put(8, List.of()); defMap.put(8, List.of());
        succMap.put(9, List.of(10)); useMap.put(9, List.of('r')); defMap.put(9, List.of('x'));
        succMap.put(10, List.of(11)); useMap.put(10, List.of('s', 'x')); defMap.put(10, List.of('s'));
        succMap.put(11, List.of(12, 13)); useMap.put(11, List.of('s', 'm')); defMap.put(11, List.of());
        succMap.put(12, List.of(13)); useMap.put(12, List.of('s')); defMap.put(12, List.of('m'));
        succMap.put(13, List.of(14)); useMap.put(13, List.of('r')); defMap.put(13, List.of('r'));
        succMap.put(14, List.of(6)); useMap.put(14, List.of()); defMap.put(14, List.of());
        succMap.put(15, List.of()); useMap.put(15, List.of('m')); defMap.put(15, List.of());

    }

    public static void main(String[] args)
    {
        while(!isSame())
        {
            IntStream.range(1, 16).forEach(i ->
            {
                oldInMap.put(i, inMap.get(i));
                oldOutMap.put(i, outMap.get(i));

                TreeSet<Character> newInSet = new TreeSet<>(useMap.get(i));
                TreeSet<Character> newOutSet = new TreeSet<>();
                // update in set
                newInSet.addAll(diffSet(outMap.get(i), defMap.get(i)));
                succMap.get(i).forEach(s -> newOutSet.addAll(inMap.get(s)));

                inMap.put(i, newInSet);
                outMap.put(i, newOutSet);
            });
        }

        System.out.println(inMap);
        System.out.println(outMap);
    }

    private static TreeSet<Character> diffSet(TreeSet<Character> s1, List<Character> s2)
    {
        TreeSet<Character> characterTreeSet = new TreeSet<>(s1);
        s2.forEach(characterTreeSet::remove);
        return characterTreeSet;
    }

    private static boolean isSame()
    {
        for(int i = 1; i <= 15; i++)
        {
            if(!inMap.get(i).equals(oldInMap.get(i))
                    || !outMap.get(i).equals(oldOutMap.get(i)))
                return false;
        }
        return true;
    }
}