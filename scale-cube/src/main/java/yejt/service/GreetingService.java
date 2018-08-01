package yejt.service;

import io.scalecube.services.annotations.Service;
import io.scalecube.services.annotations.ServiceMethod;
import yejt.model.request.GreetingRequest;
import yejt.model.response.GreetingResponse;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

@Service
public interface GreetingService
{
    @ServiceMethod
    Mono<GreetingResponse> greeting(GreetingRequest request);

    @ServiceMethod
    Flux<GreetingResponse> greetingStream(GreetingRequest request);
}
