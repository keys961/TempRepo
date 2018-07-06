package org.yejt.vocabularysearch;

import org.mybatis.spring.annotation.MapperScan;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.builder.SpringApplicationBuilder;
import org.springframework.boot.web.support.SpringBootServletInitializer;
import org.springframework.web.servlet.config.annotation.EnableWebMvc;

@MapperScan("org.yejt.vocabularysearch.mapper")
@SpringBootApplication
public class VocabularySearchApplication extends SpringBootServletInitializer
{
    @Override
    protected SpringApplicationBuilder configure(SpringApplicationBuilder application)
    {
        return application.sources(VocabularySearchApplication.class);
    }

    public static void main(String[] args)
    {
        SpringApplication.run(VocabularySearchApplication.class, args);
    }
}
