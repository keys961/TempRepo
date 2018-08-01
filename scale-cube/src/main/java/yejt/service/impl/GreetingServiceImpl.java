package yejt.service.impl;

import yejt.model.request.GreetingRequest;
import yejt.model.response.GreetingResponse;
import yejt.service.GreetingService;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

public class GreetingServiceImpl implements GreetingService
{
    @Override
    public Mono<GreetingResponse> greeting(GreetingRequest request)
    {
        return Mono.just(new GreetingResponse("Hello " + request.getRequest()));
    }

    @Override
    public Flux<GreetingResponse> greetingStream(GreetingRequest request)
    {
        return Flux.just(new GreetingResponse("Hello " + request.getRequest()),
                new GreetingResponse("XX " + request.getRequest()));
    }
}
