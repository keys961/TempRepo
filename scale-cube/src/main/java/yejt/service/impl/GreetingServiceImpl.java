package yejt.service.impl;

import yejt.model.request.GreetingRequest;
import yejt.model.response.GreetingResponse;
import yejt.service.GreetingService;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

import java.util.concurrent.atomic.AtomicInteger;
import java.util.logging.Logger;

public class GreetingServiceImpl implements GreetingService
{
    AtomicInteger time = new AtomicInteger(0);

    @Override
    public Mono<GreetingResponse> greeting(GreetingRequest request)
    {
        return Mono.just(new GreetingResponse("Hello " + request.getRequest()));
    }

    @Override
    public Flux<GreetingResponse> greetingStream(GreetingRequest request)
    {
        int[] waitingTimes = {100, 1000, 1100};
        int t = time.getAndIncrement();
        try
        {
            Thread.sleep(waitingTimes[t % 3]);
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }

        System.out.println("Fucked.." + t);

        return Flux.create(emitter ->
            {
                for(int i = 0; i < 10; i++)
                {
                    emitter.next(new GreetingResponse(t + " Hello: " + i));
                }
            });
    }
}
