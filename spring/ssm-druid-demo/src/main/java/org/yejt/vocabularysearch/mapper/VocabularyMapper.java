package org.yejt.vocabularysearch.mapper;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Select;
import org.yejt.vocabularysearch.entity.Vocabulary;

import java.util.List;

public interface VocabularyMapper
{
    Vocabulary findVocabularyByWord(String word);

    @Select("SELECT * FROM vocabulary")
    List<Vocabulary> findAll();
}
