package com.example.demo.repository;

import com.example.demo.entity.Person;
import org.springframework.boot.autoconfigure.security.SecurityProperties;
import org.springframework.data.neo4j.annotation.Query;
import org.springframework.data.neo4j.repository.Neo4jRepository;
import org.springframework.data.repository.query.Param;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;
import java.util.Set;

public interface UserRepository extends Neo4jRepository<Person, Long>
{
    /**
     * Derived method(finder) from Spring Data
     */
    Person findByName(String name);

    @Query("MATCH (a:Person {name: {name}}) -[:KNOWS]-> (b:Person) " +
            "RETURN b")
    //@Transactional: 可加上以声明事务——最好是再Service层,因为Neo4j本身是事务型的
    List<Person> getKnowsPersonList(@Param("name") String name);
}
