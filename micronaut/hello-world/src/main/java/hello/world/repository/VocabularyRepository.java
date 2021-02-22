package hello.world.repository;

import hello.world.entity.Vocabulary;
import io.micronaut.spring.tx.annotation.Transactional;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.criterion.Restrictions;

import javax.inject.Inject;
import javax.inject.Singleton;
import java.util.List;

@Singleton
@Transactional
public class VocabularyRepository
{
    @Inject
    private SessionFactory sessionFactory;

    @SuppressWarnings("unchecked")
    public Vocabulary getVocabulary(String word)
    {
        Session session = sessionFactory.openSession();
        List<Vocabulary> list = session.createCriteria(Vocabulary.class)
                .add(Restrictions.eq("word", word))
                .list();
        session.close();
        return list.get(0);
    }
}
