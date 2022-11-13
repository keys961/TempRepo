#ifndef COROUTINE_GENERATOR_H
#define COROUTINE_GENERATOR_H

#include <iostream>
#include <thread>
#include <coroutine>
#include <future>
#include <list>
#include <type_traits>

template<typename T>
struct Generator {

    // 协程执行完成之后，外部读取值时抛出的异常
    class ExhaustedException: std::exception { };

    struct promise_type {
        T value; // add a value for that promise
        bool is_ready = false;

        // 开始执行时直接挂起等待外部调用 resume 获取下一个值
        std::suspend_always initial_suspend() {
            std::cout << "Generator initial_suspend" << std::endl;
            return {};
        };

        std::suspend_always await_transform(T v) {
            std::cout << "Generator await_transform: " << v << std::endl;
            value = v;
            is_ready = true;
            return {};
        }

        std::suspend_always yield_value(T v) {
            std::cout << "Generator yield_value: " << v << std::endl;
            value = v;
            is_ready = true;
            return {};
        }

        // 执行结束后需要挂起
        // 让 Generator 来销毁
        std::suspend_always final_suspend() noexcept {
            std::cout << "Generator initial_suspend" << std::endl;
            return {};
        }

        // 为了简单，我们认为序列生成器当中不会抛出异常，这里不做任何处理
        void unhandled_exception() { }

        // 构造协程的返回值类型
        Generator get_return_object() {
            std::cout << "Generator get_return_object" << std::endl;
            // get current coroutine handle
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // 没有返回值
        void return_void() {
            std::cout << "Generator return_void" << std::endl;
        }

    };

    std::coroutine_handle<promise_type> handle;

    explicit Generator(std::coroutine_handle<promise_type> handle) noexcept
            : handle(handle) {}

    Generator(Generator &&generator) noexcept
            : handle(std::exchange(generator.handle, {})) {}

    Generator(Generator &) = delete;
    Generator &operator=(Generator &) = delete;

    ~Generator() {
        if (handle) handle.destroy();
    }

    bool has_next() {
        // 协程已经执行完成
        // but handle may destructed, so we need to final suspend
        if (handle.done()) {
            return false;
        }

        // 协程还没有执行完成，并且下一个值还没有准备好
        if (!handle.promise().is_ready) {
            handle.resume();
        }

        if (handle.done()) {
            // 恢复执行之后协程执行完，这时候必然没有通过 co_await 传出值来
            return false;
        } else {
            return true;
        }
    }

    T next() {
        if (has_next()) {
            // 此时一定有值，is_ready 为 true
            // 消费当前的值，重置 is_ready 为 false
            handle.promise().is_ready = false;
            return handle.promise().value;
        }
        throw ExhaustedException();
    }

    Generator static from_array(T array[], int n) {
        for (int i = 0; i < n; ++i) {
            co_yield array[i];
        }
    }

    Generator static from_list(std::list<T> list) {
        for (auto t: list) {
            co_yield t;
        }
    }

    Generator static from(std::initializer_list<T> args) {
        for (auto t: args) {
            co_yield t;
        }
    }

    template<typename ...TArgs>
    Generator static from(TArgs ...args) {
        (co_yield args, ...);
    }

    template<typename U>
    Generator<U> map(std::function<U(T)> f) {
        // 判断 this 当中是否有下一个元素
        while (has_next()) {
            // 使用 next 读取下一个元素
            // 通过 f 将其变换成 U 类型的值，再使用 co_yield 传出
            co_yield f(next());
        }
    }

    template<typename F>
    Generator<std::invoke_result_t<F, T>> map(F f) {
        while (has_next()) {
            co_yield f(next());
        }
    }

    template<typename F>
    // 返回值类型就是 F 的返回值类型
    std::invoke_result_t<F, T> flat_map(F f) {
        while (has_next()) {
            // 值映射成新的 Generator
            auto generator = f(next());
            // 将新的 Generator 展开
            while (generator.has_next()) {
                co_yield generator.next();
            }
        }
    }

    template<typename F>
    void for_each(F f) {
        while (has_next()) {
            f(next());
        }
    }

    template<typename R, typename F>
    R fold(R initial, F f) {
        R acc = initial;
        while (has_next()) {
            acc = f(acc, next());
        }
        return acc;
    }

    T sum() {
        T sum = 0;
        while (has_next()) {
            sum += next();
        }
        return sum;
    }

    template<typename F>
    Generator filter(F f) {
        while (has_next()) {
            T value = next();
            if (f(value)) {
                co_yield value;
            }
        }
    }

    Generator take(int n) {
        int i = 0;
        while (i++ < n && has_next()) {
            co_yield next();
        }
    }

    template<typename F>
    Generator take_while(F f) {
        while (has_next()) {
            T value = next();
            if (f(value)) {
                co_yield value;
            } else {
                break;
            }
        }
    }


};
#endif //COROUTINE_GENERATOR_H
