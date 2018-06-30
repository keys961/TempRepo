package com.example.demo.config;

import org.neo4j.ogm.session.SessionFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.context.annotation.Configuration;
import org.springframework.data.neo4j.repository.config.EnableNeo4jRepositories;
import org.springframework.data.neo4j.transaction.Neo4jTransactionManager;
import org.springframework.transaction.annotation.EnableTransactionManagement;

@Configuration
@EnableTransactionManagement
@ComponentScan("com.example.demo")
@EnableNeo4jRepositories("com.example.demo.repository")
public class Neo4jConfig
{
    //Using application.yml properties
    @Value("${spring.data.neo4j.uri}")
    private String databaseUrl;

    @Value("${spring.data.neo4j.username}")
    private String userName;

    @Value("${spring.data.neo4j.password}")
    private String password;

    @Bean
    public SessionFactory sessionFactory()
    {
        return new SessionFactory(configuration(),"com.example.demo.entity");
    }

    @Bean
    public Neo4jTransactionManager transactionManager() throws Exception
    {
        return new Neo4jTransactionManager(sessionFactory());
    }


    private org.neo4j.ogm.config.Configuration configuration()
    {
        // Enable bolt driver, but not embedded...
        return new org.neo4j.ogm.config.Configuration.Builder()
                .uri(databaseUrl)
                .credentials(userName, password)
                .build();
    }
}
