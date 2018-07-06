package org.yejt.vocabularysearch.repository;

import org.springframework.data.repository.CrudRepository;
import org.yejt.vocabularysearch.entity.Vocabulary;

public interface VocabularyRepository extends CrudRepository<Vocabulary, Long>
{
    Vocabulary findVocabularyByWord(String word);
}
