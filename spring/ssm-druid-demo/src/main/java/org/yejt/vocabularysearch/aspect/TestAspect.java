package org.yejt.vocabularysearch.aspect;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.springframework.stereotype.Component;

import java.lang.reflect.Method;

@Component
@Aspect
public class TestAspect
{
    @Before(value = "execution(* *.searchVocabulary(*))")
    public void doPre()
    {
        System.out.println("Before...");
    }

    @Around(value = "@annotation(org.springframework.web.bind.annotation.GetMapping)")
    public Object doAround(ProceedingJoinPoint point) throws Throwable
    {
        System.out.println(point.getSignature().getName());
        return point.proceed();
    }

}
