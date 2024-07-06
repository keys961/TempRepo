package org.example;

public class Main {
    public static void main(String[] args) throws Exception {
        System.out.println("Hello world!");
        Thread t = Thread.ofVirtual().start(() -> System.out.println("fuck"));
        t.join();
        String str = "";
    }
}
