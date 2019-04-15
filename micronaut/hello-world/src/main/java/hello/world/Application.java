package hello.world;

import io.micronaut.runtime.Micronaut;
import io.netty.util.internal.SystemPropertyUtil;

public class Application
{
    public static void main(String[] args)
    {
        //Micronaut.run(Application.class);
        System.out.println(SystemPropertyUtil.get("os.name"));
    }
}