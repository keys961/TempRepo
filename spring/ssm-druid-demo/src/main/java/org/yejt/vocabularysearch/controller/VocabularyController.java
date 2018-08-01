package org.yejt.vocabularysearch.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.*;
import org.yejt.vocabularysearch.entity.User;
import org.yejt.vocabularysearch.entity.Vocabulary;
import org.yejt.vocabularysearch.mapper.VocabularyMapper;
import org.yejt.vocabularysearch.repository.VocabularyRepository;

import java.util.List;

@Controller
public class VocabularyController
{
    @Autowired
    private VocabularyRepository vocabularyRepository;

    @Autowired
    private VocabularyMapper vocabularyMapper;

    @GetMapping(value = "/")
    public String home()
    {
        return "index";
    }

    @PostMapping(value = "/search", produces = "application/json",
            consumes = "application/json;charset=utf-8")
    public ResponseEntity<Vocabulary> searchVocabulary(@RequestBody String word)
    {
        Vocabulary vocabulary = vocabularyRepository.findVocabularyByWord(word);
        List<Vocabulary> vocabularies = vocabularyMapper.findAll();
        if(vocabulary == null)
            return ResponseEntity.notFound().build();

        return ResponseEntity.ok(vocabulary);
    }

    @GetMapping("/vocabulary/{id}")
    public String vocabularyPage(@PathVariable Long id, Model model)
    {
        Vocabulary vocabulary = vocabularyRepository.findOne(id);
        model.addAttribute("word", vocabulary);

        return "vocabulary";
    }

    @PostMapping(value = "/login", produces = "application/json",
            consumes = "application/json;charset=utf-8")
    public ResponseEntity<User> testPost(@RequestBody User user)
    {
        user.setPassword("123456");
        user.setUsername("654321");

        return ResponseEntity.ok(user);
    }
}
