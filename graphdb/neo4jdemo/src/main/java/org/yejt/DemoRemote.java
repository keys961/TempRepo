package org.yejt;

import org.neo4j.driver.v1.*;

import static org.neo4j.driver.v1.Values.parameters;

/**
 * Demo for CRUD on remote neo4j database
 * Can also use JDBC to implement this
 */
public class DemoRemote
{
    private static Driver driver = null;

    public static void main(String[] args)
    {
        driver = GraphDatabase.driver( "bolt://localhost:7687",
                AuthTokens.basic( "neo4j", "123456" ) );
        // Session: 一系列事务的容器，不是线程安全的
        Session session = driver.session();
        // Transaction - session.run(): 一个Auto-commit 事务，仅推荐调试使用
        session.run( "CREATE (a:Person {name: {name}, title: {title}})",
                parameters( "name", "Arthur002", "title", "King001" ) );
        addPerson("fucker", "asshole");
        StatementResult result = session.run( "MATCH (a:Person)" +
                        "RETURN a.name AS name, a.title AS title");
        while ( result.hasNext() )
        {
            Record record = result.next();
            System.out.println( record.get( "title" ).asString() + " " + record.get( "name" ).asString() );
        }
        session.close();
        driver.close();
    }

    private static void addPerson(final String name, final String title)
    {
        try (Session session = driver.session())
        {
            session.writeTransaction(
                    // Transaction - transaction function: 定义事务函数(进行一系列操作，最后会被提交)，生产中常用
                    new TransactionWork<Integer>()
                    {
                        @Override
                        public Integer execute(Transaction tx)
                        {
                            createPersonNode(tx, name, title);

                            return createPersonNode(tx, title, name);
                        }
            });
        }
    }

    private static int createPersonNode(Transaction tx, String name, String title)
    {
        tx.run( "CREATE (a:Person {name: $name, title: $title})",
                parameters( "name", name, "title", title));
        return 1;
    }
}
