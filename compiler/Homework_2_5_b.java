import java.util.*;

public class Homework_2_5_b
{
    private class Edge
    {
        int start;
        int end;
        char value;
        public Edge(int start, int end, char value)
        {
            this.start = start;
            this.end = end;
            this.value = value;
        }
    }

    private class Tuple
    {
        Set<Integer> state;
        char input;
        Set<Integer> anotherState;

        public Tuple(Set<Integer> state, Set<Integer> anotherState, char input)
        {
            this.state = state;
            this.anotherState = anotherState;
            this.input = input;
        }

        @Override
        public String toString()
        {
            return state + " -" + input +"-> " + anotherState + "\n";
        }
    }

    private HashMap<Integer, List<Edge>> graph = new HashMap<>();

    private LinkedList<Tuple> transitionTable = new LinkedList<>();

    private Set<Set<Integer>> dStates = new HashSet<>(); //not marked state

    private Set<Set<Integer>> recordedState = new HashSet<>(); //marked state

    {
        //init node 1
        List<Edge> list = new ArrayList<>();
        list.add(new Edge(1,1,'a'));
        list.add(new Edge(1,1,'b'));
        list.add(new Edge(1,2,'a'));
        graph.put(1, list);
        //init node 2
        list = new ArrayList<>();
        list.add(new Edge(2,3,'a'));
        list.add(new Edge(2,3,'b'));
        graph.put(2, list);
        //init node 3
        list = new ArrayList<>();
        list.add(new Edge(3,4,'a'));
        list.add(new Edge(3,4,'b'));
        graph.put(3, list);

        list = new ArrayList<>();
        list.add(new Edge(4,5,'a'));
        list.add(new Edge(4,5,'b'));
        graph.put(4, list);

        list = new ArrayList<>();
        list.add(new Edge(5,6,'a'));
        list.add(new Edge(5,6,'b'));
        graph.put(5, list);
    }

    private Set<Integer> getFirstElement(Set<Set<Integer>> dStates)
    {
        int count = 0;
        if(dStates.size() == 0)
            return null;

        for(Set<Integer> set : dStates)
        {
            return set;
        }

        return null;
    }

    public List<Tuple> solve()
    {
        Set<Integer> initState = new HashSet<>();
        initState.add(1);
        dStates.add(initState);

        while(!dStates.isEmpty())
        {
            Set<Integer> t = getFirstElement(dStates);
            dStates.remove(t);
            recordedState.add(t);
            // a
            Set<Integer> u1 = getEClosure(getMove(t, 'a'));
            if(!recordedState.contains(u1))
                dStates.add(u1);
            transitionTable.add(new Tuple(t, u1, 'a'));

            Set<Integer> u2 = getEClosure(getMove(t, 'b'));
            if(!recordedState.contains(u2))
                dStates.add(u2);
            transitionTable.add(new Tuple(t, u2, 'b'));
        }

        return transitionTable;
    }


    private Set<Integer> getEClosure(Set<Integer> states)
    {
        Set<Integer> closure = new HashSet<>(states);
        Stack<Integer> stack = new Stack<>();
        for(Integer state : states)
            stack.push(state);

        while(!stack.isEmpty())
        {
            int s = stack.pop();
            List<Edge> edges = graph.get(s);
            if(edges == null)
                continue;
            for(Edge e : edges)
            {
                if(e.value == 0)
                {
                    if(!closure.contains(e.end))
                    {
                        closure.add(e.end);
                        stack.push(e.end);
                    }
                }
            }
        }

        return closure;
    }

    private  Set<Integer> getMove(Set<Integer> states, char input)
    {
        Set<Integer> moveSet = new HashSet<>();

        for(Integer state : states)
        {
            List<Edge> edges = graph.get(state);
            if(edges == null)
                continue;
            for(Edge e : edges)
            {
                if(e.value == input)
                    moveSet.add(e.end);
            }
        }

        return moveSet;
    }

    public static void main(String[] args)
    {
        List<Tuple> tupleLinkedList = new Homework_2_5_b().solve();
        System.out.println("State\t\t\t\ta\t\t\t\tb");
        for(int i = 0; i < tupleLinkedList.size(); i += 2)
        {
            System.out.print(tupleLinkedList.get(i).state );
            int elementCount = tupleLinkedList.get(i).state.size();
            elementCount = 7 - elementCount;
            for(int j = 0; j < elementCount; j++)
                System.out.print("\t");
            System.out.print(tupleLinkedList.get(i).anotherState);

            elementCount = tupleLinkedList.get(i).anotherState.size();
            elementCount = 7 - elementCount;
            for(int j = 0; j < elementCount; j++)
                System.out.print("\t");
            System.out.print(tupleLinkedList.get(i + 1).anotherState + "\n");

        }

    }
}
