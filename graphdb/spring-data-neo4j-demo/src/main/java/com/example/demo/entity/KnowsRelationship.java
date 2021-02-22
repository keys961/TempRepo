package com.example.demo.entity;

import org.neo4j.ogm.annotation.*;

@RelationshipEntity(type = "KNOWS")
public class KnowsRelationship
{
    @Id
    @GeneratedValue
    private Long id;

    @StartNode
    private Person startPerson;

    @EndNode
    private Person endPerson;

    public Person getEndPerson()
    {
        return endPerson;
    }

    public void setEndPerson(Person endPerson)
    {
        this.endPerson = endPerson;
    }

    public Person getStartPerson()
    {
        return startPerson;
    }

    public void setStartPerson(Person startPerson)
    {
        this.startPerson = startPerson;
    }

    public Long getId()
    {
        return id;
    }

    public void setId(Long id)
    {
        this.id = id;
    }
}
