package yejt;

import io.scalecube.services.Microservices;
import io.scalecube.transport.Address;
import yejt.service.GreetingService;
import yejt.service.impl.GreetingServiceImpl;

public class Main
{
    public static void main(String[] args) throws InterruptedException
    {
        //2. Construct a ScaleCube node which joins the cluster hosting the Greeting Service
        GreetingService service = new GreetingServiceImpl();
        Microservices microservices = Microservices.builder()
                .discovery(builder -> {
                    builder.seeds(Address.from("192.168.211.1:4801"));
                })// some address so its possible to join the cluster.
                .services(service)
                .servicePort(4801)
                .startAwait();
        System.out.println("Current: " + microservices.discovery().address().toString());

        Thread.currentThread().join();
    }
}
