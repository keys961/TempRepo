package hello.world.entity;

import javax.persistence.*;

@Entity
@Table(name = "Vocabulary")
public class Vocabulary
{
    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    @Column(name = "word", nullable = false)
    private String word;

    @Column(name = "translation")
    private String translation;

    @Column(name = "phonetic")
    private String phonetic;

    @Column(name = "tag")
    private String tag;

    @Column(name = "category")
    private String category;

    public Vocabulary(){}

    public Vocabulary(String word, String translation, String phonetic, String tag, String category)
    {
        this.word = word;
        this.translation = translation;
        this.phonetic = phonetic;
        this.tag = tag;
        this.category = category;
    }

    public Long getId()
    {
        return id;
    }

    public void setId(Long id)
    {
        this.id = id;
    }

    public String getWord()
    {
        return word;
    }

    public void setWord(String word)
    {
        this.word = word;
    }

    public String getTranslation()
    {
        return translation;
    }

    public void setTranslation(String translation)
    {
        this.translation = translation;
    }

    public String getPhonetic()
    {
        return phonetic;
    }

    public void setPhonetic(String phonetic)
    {
        this.phonetic = phonetic;
    }

    public String getTag()
    {
        return tag;
    }

    public void setTag(String tag)
    {
        this.tag = tag;
    }

    public String getCategory()
    {
        return category;
    }

    public void setCategory(String category)
    {
        this.category = category;
    }

    @Override
    public int hashCode()
    {
        return this.id.hashCode();
    }

    @Override
    public boolean equals(Object obj)
    {
        return (obj instanceof Vocabulary)
                && (((Vocabulary) obj).id.equals(this.id))
                && (((Vocabulary) obj).word.equals(this.word));
    }
}
