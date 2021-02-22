package org.yejt;

import org.neo4j.graphdb.*;
import org.neo4j.graphdb.factory.GraphDatabaseFactory;

import java.io.File;

/**
 * Demo for CRUD on local embedded neo4j database
 */
public class Demo
{
    private static final File DB_PATH = new File("C:\\Users\\keys9\\Documents\\Projects\\Demo\\data");
    private static GraphDatabaseService graphDb;

    private static void registerShutdownHook(final GraphDatabaseService graphDb)
    {
        Runtime.getRuntime().addShutdownHook(new Thread(graphDb::shutdown));
    }

    private static enum RelTypes implements RelationshipType
    {
        RELEASED,
        FUCKED
    }

    @SuppressWarnings("unused")
    private static void addData()
    {
        Node node1;
        Node node2;
        Label label1;
        Label label2;
        Relationship relationship;

        try (Transaction tx = graphDb.beginTx())
        {
            // 创建标签
            label1 = Label.label("Man");
            label2 = Label.label("Woman");
            // 创建节点
            node1 = graphDb.createNode(label1);
            node1.setProperty("name", "I");
            node2 = graphDb.createNode(label2);
            node2.setProperty("name", "XXX");
            // 创建关系及属性
            relationship = node1.createRelationshipTo(node2, RelTypes.FUCKED);
            relationship.setProperty("date", "2001-09-14");
            // 结果输出
            System.out.println("created node name is " + node1.getProperty("name"));
            System.out.println(relationship.getProperty("date"));
            System.out.println("created node name is " + node2.getProperty("name"));
            // 提交事务
            tx.success();
        }
        graphDb.shutdown();
    }

    @SuppressWarnings("unused")
    private static void queryAndUpdate()
    {
        try (Transaction tx = graphDb.beginTx())
        {
            // 查询节点
            Label label = Label.label("Man");
            Node node = graphDb.findNode(label, "name", "I");
            System.out.println("query node name is " + node.getProperty("name"));
            // 更新节点
            node.setProperty("birthday", "1979-01-18");
            System.out
                    .println(node.getProperty("name") + "'s birthday is " + node.getProperty("birthday", ""));
            // 提交事务
            tx.success();
        }
        graphDb.shutdown();
    }

    @SuppressWarnings("unused")
    private static void delete()
    {
        try (Transaction tx = graphDb.beginTx())
        {
            // 获得节点
            Label label = Label.label("Woman");
            Node node = graphDb.findNode(label, "name", "XXX");
            // 获得关系
            Relationship relationship = node.getSingleRelationship(RelTypes.FUCKED, Direction.INCOMING);
            // 删除关系和节点
            relationship.delete();
            relationship.getStartNode().delete();
            node.delete();
            tx.success();
        }
        graphDb.shutdown();
    }

    public static void main(String[] args)
    {
        graphDb = new GraphDatabaseFactory().newEmbeddedDatabase(DB_PATH);
        registerShutdownHook(graphDb);
        addData();
        // queryAndUpdate();
        // delete();

    }
}
