package com.example.springsourcecode;

import com.google.common.util.concurrent.RateLimiter;
import lombok.extern.slf4j.Slf4j;

import java.util.concurrent.TimeUnit;

@Slf4j
public class RateLimiterApp {

    public static void main(String[] args) {
        RateLimiter rateLimiter = RateLimiter.create(2, 1, TimeUnit.SECONDS);
        rateLimiter.acquire(4);
        log.info("acquire 4");
        rateLimiter.acquire(1);
        log.info("acquire 1");
    }

}
