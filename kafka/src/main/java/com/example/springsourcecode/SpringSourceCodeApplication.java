package com.example.springsourcecode;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import sun.misc.Unsafe;

import java.util.*;
import java.util.stream.Collectors;

@SpringBootApplication
public class SpringSourceCodeApplication {

    public static void main(String[] args) {
        TreeNode root = new TreeNode(0);
        root.left = new TreeNode(1);
        root.left.left = new TreeNode(3);
        root.right = new TreeNode(2);
        Codec codec = new Codec();
        System.out.println(codec.serialize(root));
        String str = codec.serialize(root);
        TreeNode root1 = codec.deserialize(str);
        System.out.println(root1);
    }

    private static ListNode build(int[] arr) {
        ListNode head = new ListNode(0);
        ListNode cur = head;
        for(int i = 0; i < arr.length; i++) {
            cur.next = new ListNode(arr[i]);
            cur = cur.next;
        }
        return head.next;
    }

}

class Solution {

}

class Codec {

    // Encodes a tree to a single string.
    public String serialize(TreeNode root) {
        Stack<TreeNode> stack = new Stack<>();
        List<String> elements = new ArrayList<>();

        TreeNode cur = root;
        while (cur != null || !stack.isEmpty()) {
            while(cur != null) {
                elements.add(String.valueOf(cur.val));
                stack.push(cur);
                cur = cur.left;
            }
            elements.add("null");
            cur = stack.pop();
            cur = cur.right;
        }
        elements.add("null");
        StringBuilder sb = new StringBuilder();
        for(String element : elements) {
            if(sb.length() != 0) {
                sb.append(",");
            }
            sb.append(element);
        }
        return sb.toString();
    }

    // Decodes your encoded data to tree.
    public TreeNode deserialize(String data) {
        if(data.isEmpty()) {
            return null;
        }

        String[] elements = data.split(",");
        int[] index = new int[] {0};
        return helper(elements, index);
    }

    private TreeNode helper(String[] elements, int[] index) {
        if(index[0] >= elements.length) {
            return null;
        }
        if("null".equals(elements[index[0]])) {
            index[0]++;
            return null;
        }

        TreeNode node = new TreeNode(Integer.parseInt(elements[index[0]]));
        index[0]++;
        node.left = helper(elements, index);
        node.right = helper(elements, index);
        return node;
    }
}

class AllOne {

    private Map<String, Integer> storage;

    private Map<Integer, Entry> index;

    private Entry head;

    private Entry tail;

    private static class Entry {
        final int refCnt;
        final Set<String> strs = new HashSet<>();
        Entry prev;
        Entry next;

        Entry(final int refCnt) {
            this.refCnt = refCnt;
        }
    }

    /** Initialize your data structure here. */
    public AllOne() {
        this.storage = new HashMap<>();
        this.index = new HashMap<>();
        this.head = new Entry(0);
        this.tail = new Entry(Integer.MAX_VALUE);
        this.head.next = tail;
        this.tail.prev = head;
        this.index.put(0, this.head);
        this.index.put(Integer.MAX_VALUE, this.tail);
    }

    /** Inserts a new key <Key> with value 1. Or increments an existing key by 1. */
    public void inc(String key) {
        if(storage.containsKey(key)) {
            int originRefCnt = storage.get(key);
            storage.put(key, originRefCnt + 1);
            // update
            int newRefCnt = originRefCnt + 1;
            if(index.containsKey(newRefCnt)) {
                index.get(newRefCnt).strs.add(key);
            } else {
                Entry entry = addNewEntry(originRefCnt, newRefCnt);
                entry.strs.add(key);
            }
            index.get(originRefCnt).strs.remove(key);
            if(index.get(originRefCnt).strs.isEmpty()) {
                removeOldEntry(originRefCnt);
            }
        } else {
            // ref cnt = 1
            storage.put(key, 1);
            if(index.containsKey(1)) {
                index.get(1).strs.add(key);
            } else {
                Entry entry = addNewEntry(0, 1);
                entry.strs.add(key);
            }
        }
    }

    /** Decrements an existing key by 1. If Key's value is 1, remove it from the data structure. */
    public void dec(String key) {
        if(storage.containsKey(key)) {
            int originRefCnt = storage.get(key);
            int newRefCnt = originRefCnt - 1;
            if(originRefCnt > 1) {
                storage.put(key, originRefCnt - 1);
            } else {
                storage.remove(key);
            }

            if(newRefCnt > 0) {
                if(index.containsKey(newRefCnt)) {
                    index.get(newRefCnt).strs.add(key);
                } else {
                    Entry entry = addNewEntry(originRefCnt, newRefCnt);
                    entry.strs.add(key);
                }
            }

            index.get(originRefCnt).strs.remove(key);
            if(index.get(originRefCnt).strs.isEmpty()) {
                removeOldEntry(originRefCnt);
            }
        }
    }

    /** Returns one of the keys with maximal value. */
    public String getMaxKey() {
        if(storage.isEmpty()) {
            return "";
        }

        return this.tail.prev.strs.iterator().next();
    }

    /** Returns one of the keys with Minimal value. */
    public String getMinKey() {
        if(storage.isEmpty()) {
            return "";
        }

        return this.head.next.strs.iterator().next();
    }

    private Entry addNewEntry(int originRefCnt, int newRefCnt) {
        Entry entry = new Entry(newRefCnt);
        if(originRefCnt < newRefCnt) {
            // origin -> new
            Entry prev = index.get(originRefCnt);

            entry.prev = prev;
            entry.next = prev.next;

            prev.next.prev = entry;
            prev.next = entry;
        } else {
            // new -> origin
            Entry next = index.get(originRefCnt);
            entry.prev = next.prev;
            entry.next = next;
            next.prev.next = entry;
            next.prev = entry;
        }
        index.put(newRefCnt, entry);
        return entry;
    }

    private void removeOldEntry(int refCnt) {
        if(this.index.containsKey(refCnt)) {
            Entry entry = this.index.get(refCnt);
            entry.prev.next = entry.next;
            entry.next.prev = entry.prev;

            entry.prev = null;
            entry.next = null;
            this.index.remove(refCnt);
        }
    }
}

class Node {
    public int val;
    public Node left;
    public Node right;

    public Node() {}

    public Node(int _val) {
        val = _val;
    }

    public Node(int _val,Node _left,Node _right) {
        val = _val;
        left = _left;
        right = _right;
    }
};

class ListNode {
     int val;
     ListNode next;
     ListNode(int x) { val = x; }
}

class TreeNode {
      int val;
      TreeNode left;
      TreeNode right;
      TreeNode(int x) { val = x; }
}

class NumArray {

    private int[] tree;

    private int n;

    public NumArray(int[] nums) {
        n = nums.length;
        tree = new int[2 * nums.length - 1];
        buildTree(nums, 0, nums.length - 1, 0);
    }

    private void buildTree(int[] nums, int start, int end, int i) {
        if(start > end) {
            return;
        }

        if(start == end) {
            tree[i] = nums[start];
        } else {
            int mid = (start + end) / 2;
            buildTree(nums, start, mid, 2 * i + 1);
            buildTree(nums, mid + 1, end, 2 * i + 2);
            tree[i] = tree[2 * i + 1] + tree[2 * i + 2];
        }
    }

    public void update(int i, int val) {
        // i + n => leaf
        int diff = val - tree[i + n];
        int node = i + n;
        while(node != 0) {
            tree[node] += diff;
            node /= 2;
        }
        tree[0] += diff;
    }

    public int sumRange(int i, int j) {
        return query(0, n - 1, 0, i, j);
    }

    private int query(int start, int end, int root, int i, int j) {
        if(start == i && end == j) {
            return tree[root];
        }
        int mid = (start + end) / 2;
        if(i > mid) {
            return query(mid + 1, end, root * 2 + 2, i, j);
        }
        if(j < mid) {
            return query(start, mid - 1, root * 2 + 1, i, j);
        }

        return query(start, mid, root * 2 + 1, i, mid)
                + query(mid + 1, end, root * 2 + 1, mid + 1, j);
    }
}

class Solution1 {
    public boolean isCompleteTree(TreeNode root) {
        if(root == null) {
            return true;
        }
        LinkedList<TreeNode> queue = new LinkedList<>();
        LinkedList<TreeNode> levelNodes = new LinkedList<>();
        int cnt = 0;
        int level = 0;

        queue.add(root);
        while(!queue.isEmpty()) {
            TreeNode node = queue.pollFirst();
            cnt++;
            if(node != null) {
                queue.add(node.left);
                queue.add(node.right);
            }
            levelNodes.add(node);
            if(cnt == (1 << level)) {
                if(!check(levelNodes, level)) {
                    return false;
                }
                cnt = 0;
                level++;
                levelNodes.clear();
            }
        }
        return true;
    }

    private boolean check(List<TreeNode> levelNodes, int level) {
        boolean nullNode = false;
        boolean allNull = true;
        int cnt = 0;
        for(TreeNode node : levelNodes) {
            if(node == null) {
                nullNode = true;
            } else {
                cnt++;
                allNull = false;
            }
        }
        if(allNull) {
            return true;
        }
        return cnt == (1 << level);
    }
}
