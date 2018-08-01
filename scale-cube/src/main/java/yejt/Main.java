package yejt;

import io.scalecube.services.Microservices;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import yejt.model.request.GreetingRequest;
import yejt.model.response.GreetingResponse;
import yejt.service.GreetingService;
import yejt.service.impl.GreetingServiceImpl;

public class Main
{
    public static void main(String[] args) throws InterruptedException
    {
        //1. ScaleCube Node node with no members
        Microservices seed = Microservices.builder().startAwait();

        //2. Construct a ScaleCube node which joins the cluster hosting the Greeting Service
        Microservices microservices = Microservices.builder()
                .seeds(seed.cluster().address()) // some address so its possible to join the cluster.
                .services(new GreetingServiceImpl())
                .startAwait();

        Microservices consumer = Microservices.builder()
                .seeds(seed.cluster().address())
                .startAwait();

        //3. Create service proxy
        GreetingService service = consumer.call().create().api(GreetingService.class);

        // Execute the services and subscribe to service events
        // Call service and when complete print the greeting.
        GreetingRequest req = new GreetingRequest("Joe");

        Publisher<GreetingResponse> publisher = service.greeting(req);

        Mono.from(publisher).subscribe(result -> {
                    System.out.println(result.getResponse());
                });

        service.greetingStream(req)
                .subscribe(onNext -> {
                    System.out.println(onNext.getResponse());
                });

        Thread.currentThread().join();
    }
}
