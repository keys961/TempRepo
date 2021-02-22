package yejt;

import io.scalecube.services.Microservices;
import io.scalecube.transport.Address;
import org.reactivestreams.Publisher;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;
import yejt.model.request.GreetingRequest;
import yejt.model.response.GreetingResponse;
import yejt.route.DefaultRouter;
import yejt.service.GreetingService;

import java.time.Duration;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeoutException;

public class Consumer
{
    public static void main(String[] args) throws InterruptedException
    {
        Microservices consumer = Microservices.builder()
                .discovery(builder -> {
                    builder.seeds(Address.from("192.168.211.1:4801"));
                })// some address so its possible to join the cluster.
                .startAwait();
        //3. Create service proxy
        GreetingService service = consumer.call().router(new DefaultRouter()).
                create().api(GreetingService.class);

        // Execute the services and subscribe to service events
        // Call service and when complete print the greeting.
        GreetingRequest req = new GreetingRequest("Joe");

        Publisher<GreetingResponse> publisher = service.greeting(req);

        Mono.from(publisher).subscribe(result -> {
            System.out.println(result.getResponse());
        });
        final CountDownLatch[] countDownLatch = {new CountDownLatch(10)};

        Flux<GreetingResponse> responseFlux = service.greetingStream(req);

        responseFlux
                .timeout(Duration.ofSeconds(1), Mono.error(new TimeoutException("timeout exception")))
                .retry(3, throwable -> {
                    if(throwable instanceof TimeoutException) {
                        if (countDownLatch[0].getCount() > 0) {
                            System.err.println("Timeout, retrying...");
                            if (countDownLatch[0].getCount() != 10) {
                                countDownLatch[0] = new CountDownLatch(10);
                            }
                            return true;
                        }
                    }
                    return false;
                })
                .subscribe(onNext -> {
                    System.out.println(onNext.getResponse());
                    countDownLatch[0].countDown();
                }, onError -> {
                    long count = countDownLatch[0].getCount();
                    if(!(onError instanceof TimeoutException) || count > 0) {
                        System.err.println(onError.getMessage());
                    }
                    // 要直接返回
                    while(count-- > 0) {
                        countDownLatch[0].countDown();
                    }
                });
        countDownLatch[0].await();
    }

    private static void printThread()
    {
        ThreadGroup group = Thread.currentThread().getThreadGroup();
        ThreadGroup topGroup = group;
        // 遍历线程组树，获取根线程组
        while (group != null) {
            topGroup = group;
            group = group.getParent();
        }
        // 激活的线程数再加一倍，防止枚举时有可能刚好有动态线程生成
        int slackSize = topGroup.activeCount() * 2;
        Thread[] slackThreads = new Thread[slackSize];
        // 获取根线程组下的所有线程，返回的actualSize便是最终的线程数
        int actualSize = topGroup.enumerate(slackThreads);
        Thread[] atualThreads = new Thread[actualSize];
        // 复制slackThreads中有效的值到atualThreads
        System.arraycopy(slackThreads, 0, atualThreads, 0, actualSize);
        System.out.println("Threads size is " + atualThreads.length);
        for (Thread thread : atualThreads) {
            System.out.println("Thread name : " + thread.getName());
        }
    }
}
