package org.yejt.vocabularysearch.mapper;

import org.junit.Assert;
import org.junit.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.yejt.vocabularysearch.BaseTest;
import org.yejt.vocabularysearch.entity.Vocabulary;

import java.util.List;

public class VocabularyMapperTest extends BaseTest
{
    @Autowired
    private VocabularyMapper mapper;

    @Test
    public void mapperTest()
    {
        Vocabulary vocabulary = mapper.findVocabularyByWord("hello");
        Assert.assertNotNull(vocabulary);
        List<Vocabulary> vocabularies = mapper.findAll();
        Assert.assertTrue(!vocabularies.isEmpty());
        System.out.print("sadsadasdsa\n");
    }
}
