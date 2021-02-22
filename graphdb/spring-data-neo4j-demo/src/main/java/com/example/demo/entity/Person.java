package com.example.demo.entity;

import org.neo4j.ogm.annotation.*;

import java.util.HashSet;
import java.util.Set;

@NodeEntity
public class Person
{
    @Id
    @GeneratedValue
    private Long id;

    private String name;

    private String title;

//    @Relationship(type = "KNOWS")
//    private Set<Person> knowSet;

    @Relationship(type = "KNOWS")
    //knowSet会置null, knownSet也是
    //找不到联系也会让集合变为null
    private Set<KnowsRelationship> knowsRelationshipSet = new HashSet<>(1);

//    @Relationship(type = "KNOWS", direction = Relationship.INCOMING)
//    private Set<Person> knownSet;

    @Relationship(type = "KNOWS", direction = Relationship.INCOMING)
    private Set<KnowsRelationship> knownRelationshipSet = new HashSet<>(1);

    public Long getId()
    {
        return id;
    }

    public void setId(Long id)
    {
        this.id = id;
    }

    public String getName()
    {
        return name;
    }

    public void setName(String name)
    {
        this.name = name;
    }

    public String getTitle()
    {
        return title;
    }

    public void setTitle(String title)
    {
        this.title = title;
    }

    public Set<KnowsRelationship> getKnowsRelationshipSet()
    {
        return knowsRelationshipSet;
    }

    public void setKnowsRelationshipSet(Set<KnowsRelationship> knowsRelationshipSet)
    {
        this.knowsRelationshipSet = knowsRelationshipSet;
    }

    public Set<KnowsRelationship> getKnownRelationshipSet()
    {
        return knownRelationshipSet;
    }

    public void setKnownRelationshipSet(Set<KnowsRelationship> knownRelationshipSet)
    {
        this.knownRelationshipSet = knownRelationshipSet;
    }

    //    public Set<KnowsRelationship> getKnowsRelationshipSet()
//    {
//        return knowsRelationshipSet;
//    }
//
//    public void setKnowsRelationshipSet(Set<KnowsRelationship> knowsRelationshipSet)
//    {
//        this.knowsRelationshipSet = knowsRelationshipSet;
//    }
}
