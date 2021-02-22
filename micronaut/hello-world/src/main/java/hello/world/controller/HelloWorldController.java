package hello.world.controller;

import hello.world.entity.Vocabulary;
import hello.world.repository.VocabularyRepository;
import io.micronaut.http.annotation.Controller;
import io.micronaut.http.annotation.Get;

import javax.inject.Inject;

@Controller("/")
public class HelloWorldController
{
    @Inject
    private VocabularyRepository vocabularyRepository;

    @Get("/hello")
    public String hello()
    {
        return "hello\n";
    }

    @Get("/vocabulary")
    public Vocabulary vocabulary()
    {
        return vocabularyRepository.getVocabulary("hello");
    }
}
