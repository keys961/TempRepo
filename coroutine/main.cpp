#include <iostream>
#include <thread>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "task.h"
#include "ioutils.h"
#include "generator.h"


// 1. Coroutine State
// Created, Running, Suspending, Resuming, Exception, Returning

// 2. Awaiter
//
// co_await Awaiter => Store coroutine state to Awaiter
// and suspend by await_ready() (return false)
//
// then will call await_suspend(handle) (if await_ready returns false): Can call handle.resume() to resume the coroutine
// which thread calls h.resume(), h will be scheduled on this thread, and continue h coroutine first.
//
// return void/true: suspend handle coroutine
// return false: resume handle coroutine
// return other handle: resume returning handle coroutine
// exception: resume handle coroutine and throw exception

// then coroutine resume, and will call await_resume()
// return the value for co_wait

// 3. Whether a function is a coroutine - determined by returning value
//
// Return value struct needs to have nested struct promise_type
//
// Execute the coroutine:
// 1) Call promise_type::initial_suspend(), return the awaiter
// 2) Execute the body: co_await(suspend), co_yield(suspend and return a value), co_return(return a value)
// 3) Return: call promise_type::return_value(xxx), xxx is used in co_return
// 4) Exception: call promise_type::unhandled_exception() to catch it
// 5) Finalize: After returning, call promise_type::final_suspend()


// 4. co_await expr
// If there is promise_type::await_transform, it will return promise_type::await_transform(expr), otherwise return expr
// (expr) is awaitable.
// Get awaiter (need await_ready. await_suspend and await_resume). If awaitable has co_await operator, use it.
// Or the awaitable is the awaiter.
// std::suspend_always is an awaitable/awaiter

// 6. co_yield expr
// = co_await promise_type::yield_value(expr)

Generator<int> sequence() {
    int i = 0;
    for(int p = 0; p < 5; p++) {
        // co_await Generator::promise_type.await_transform(expr)
        co_await i++;
    }
}

Generator<int> fibonacci() {
    co_yield 0;
    co_yield 1;
    int a = 0;
    int b = 1;
    while (true) {
        co_yield a + b; // fib(N), N > 1
        b = a + b;
        a = b - a;
    }
}


Task<int, AsyncExecutor> simple_task2() {
    debug("task 2 start ...");
    using namespace std::chrono_literals;
    co_await 1s;
    debug("task 2 returns after 1s.");
    co_return 2;
}

Task<int, NewThreadExecutor> simple_task3() {
    debug("in task 3 start ...");
    using namespace std::chrono_literals;
    co_await 2s;
    debug("task 3 returns after 2s.");
    co_return 3;
}

Task<int, LooperExecutor> simple_task() {
    debug("task start ...");
    auto result2 = co_await simple_task2();
    debug("returns from task2: ", result2);
    auto result3 = co_await simple_task3();
    debug("returns from task3: ", result3);
    co_return 1 + result2 + result3;
}

using namespace std::chrono_literals;

Task<void, LooperExecutor> producer(Channel<int> &channel) {
    int i = 0;
    while (i < 10) {
        debug("send: ", i);
        // 或者使用 write 函数：co_await channel.write(i++);
        co_await (channel << (i));
        co_await 300ms;
        i++;
    }

    channel.close();
    debug("close channel, exit.");
}

Task<void, LooperExecutor> consumer(Channel<int> &channel) {
    while (channel.is_active()) {
        try {
            // 或者使用 read 函数：auto received = co_await channel.read();
            int received;
            co_await (channel >> received);
            debug("receive: ", received);
            co_await 2s;
        } catch (std::exception &e) {
            debug("exception: ", e.what());
        }
    }

    debug("exit.");
}

Task<void, LooperExecutor> consumer2(Channel<int> &channel) {
    while (channel.is_active()) {
        try {
            auto received = co_await channel.read();
            debug("receive2: ", received);
            co_await 3s;
        } catch (std::exception &e) {
            debug("exception2: ", e.what());
        }
    }

    debug("exit.");
}

Task<std::string, LooperExecutor> http_get(std::string host, std::string path) {
    httplib::Client cli(host);

    // block get response
    auto res = cli.Get(path.c_str());
    debug("fetch resp finished...");
    if (res) {
        co_return res->body;
    } else {
        co_return httplib::to_string(res.error());
    }
}

Task<void, LooperExecutor> test_http() {
    try {
        // Looper thread
        debug("send request...");

        // coroutine suspended, Looper thread empty
        auto result = co_await http_get("https://api.github.com", "/users/keys961");
        // coroutine resumed by Looper thread
        debug("done.");

        auto json = nlohmann::json::parse(result);
        debug(json.dump(2));
        debug(json["login"], json["url"]);
    } catch (std::exception &e) {
        debug(e.what());
    }
}

int main() {
    // created coroutine and initial suspend sequence() function (at line #0)
    /*auto gen = sequence();
    while (gen.has_next()) {
        // call next() will resume sequence() function
        // and finally at co_await i++
        // and it will call Generator::await_transform, and suspend again
        std::cout << gen.next() << std::endl;
    }

    auto fib = fibonacci();
    for(int i = 0; i < 5; i++) {
        std::cout << fib.next() << std::endl;
    }

    auto simpleTask = simple_task();
    simpleTask.then([](int i) {
       debug("simple task end: ", i);
    }).catching([](std::exception &e) {
       debug("error occurred", e.what());
    });
    try {
        auto i = simpleTask.get_result();
        debug("simple task end from get: ", i);
    } catch (std::exception &e) {
        debug("error: ", e.what());
    }

    auto channel = Channel<int>(2);
    auto p = producer(channel);
    auto c = consumer(channel);
    auto c2 = consumer2(channel);

    // wait for coroutine finished
    p.get_result();
    c.get_result();
    c2.get_result();*/
    debug("main")
    test_http().get_result();

    return 0;
}