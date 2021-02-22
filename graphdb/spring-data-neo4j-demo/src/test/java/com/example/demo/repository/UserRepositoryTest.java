package com.example.demo.repository;

import com.example.demo.DemoApplicationTests;
import com.example.demo.entity.Person;
import org.junit.Assert;
import org.junit.Test;
import org.springframework.beans.factory.annotation.Autowired;

import java.util.List;
import java.util.Set;

public class UserRepositoryTest extends DemoApplicationTests
{
    @Autowired
    private UserRepository userRepository;

    @Test
    public void queryNodeTest()
    {
        Person person = userRepository.findByName("Ian");
        Assert.assertNotNull(person);
        List<Person> personList = userRepository.getKnowsPersonList("Jim");
        Assert.assertEquals(2, personList.size());
    }

    @Test
    public void queryRelationTest()
    {
        Person person = userRepository.findByName("Emil");

        Assert.assertEquals(2, person.getKnownRelationshipSet().size());
        Assert.assertEquals(0, person.getKnowsRelationshipSet().size());
    }

}
