package org.yejt.vocabularysearch.repository;

import org.junit.Assert;
import org.junit.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.yejt.vocabularysearch.BaseTest;
import org.yejt.vocabularysearch.entity.Vocabulary;

public class VocabularyRepositoryTest extends BaseTest
{
    @Autowired
    private VocabularyRepository vocabularyRepository;

    @Test
    public void vocabularyTest()
    {
        Vocabulary vocabulary = vocabularyRepository.findVocabularyByWord("discover");
        Assert.assertNotNull(vocabulary);
        Assert.assertEquals("discover", vocabulary.getWord());
    }

}
