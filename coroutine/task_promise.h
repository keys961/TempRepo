//
// Created by Jintao Ye on 13/11/22.
//

#ifndef COROUTINE_TASK_PROMISE_H
#define COROUTINE_TASK_PROMISE_H

#include <coroutine>
#include <mutex>
#include <optional>
#include <list>

#include "ret.h"
#include "executor.h"
#include "task_awaiter.h"
#include "sleep_awaiter.h"
#include "channel_awaiter.h"

template<typename ResultType, typename Executor>
class Task;

template<typename AwaiterImpl, typename R>
concept AwaiterImplRestriction = std::is_base_of<Awaiter<R>, AwaiterImpl>::value;

// initial suspend, will schedule the coroutine whether it will suspend upon executing co_await Task<U>
struct DispatchAwaiter {
    explicit DispatchAwaiter(AbstractExecutor *executor) noexcept
            : _executor(executor) {}

    bool await_ready() const { return false; }

    void await_suspend(std::coroutine_handle<> handle) const {
        // initially suspend current coroutine
        // resumed by the executor
        _executor->execute([handle]() {
            handle.resume();
        });
    }

    void await_resume() {}
private:
    AbstractExecutor *_executor;
};

template<typename T, typename Exec>
struct TaskPromise {
    // ~~execute the coroutine immediately~~
    // Create a dispatcher, firstly suspend, then let executor resume it
    DispatchAwaiter initial_suspend() { return DispatchAwaiter{&exec}; }

    // need to suspend to wait for the cleaning finished
    std::suspend_always final_suspend() noexcept { return {}; }

    // create the return Task<T> object
    Task<T, Exec> get_return_object() {
        return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
    }

    // store the exception to the promise if coroutine has un-caught exception
    void unhandled_exception() {
        std::lock_guard lock(completion_lock);
        result = Ret<T>(std::current_exception());
        // finished with exception, notify all
        completion.notify_all();
        // notify callbacks
        notify_callbacks();
    }

    // for co_return, to store the return value
    void return_value(T value) {
        std::lock_guard lock(completion_lock);
        result = Ret<T>(std::move(value));
        // finished, notify all
        completion.notify_all();
        // notify callbacks
        notify_callbacks();
    }

    /**
     * co_await <XXX>, where <XXX> is an awaiter.
     * Here we register the executor to the passing awaiter.
     */
    template<typename AwaiterImpl>
    requires AwaiterImplRestriction<AwaiterImpl, typename AwaiterImpl::RetType>
    AwaiterImpl await_transform(AwaiterImpl awaiter) {
        awaiter.install_executor(&exec);
        return awaiter;
    }

    // for co_await Task<U> coroutine
    // U is not same as T, since in a Task<T>, it may call co_await Task<U>
    // TaskAwaiter<U> will schedule the coroutine (whether suspend or not)

    // _Exec is not same as Exec, that means `task` is scheduled by _Exec
    // but current coroutine is scheduled by Exec
    template<typename U, typename _Exec>
    TaskAwaiter<U, _Exec> await_transform(Task<U, _Exec>&& task) {
        return await_transform(TaskAwaiter<U, _Exec>(std::move(task)));
    }

    // for co_await XXms coroutine
    // transfer XXms to SleepAwaiter

    // this coroutine will be suspended (switched out) for some time and resumed
    // SleepAwaiter will suspend current coroutine, and resume the coroutine after some time
    template<typename _Rep, typename _Period>
    SleepAwaiter await_transform(std::chrono::duration<_Rep, _Period> &&duration) {
        return await_transform(SleepAwaiter(std::move(duration)));
    }

//    // for co_await channel.read()
//    // channel.read() returns an awaiter
//    template<typename V>
//    auto await_transform(ReadAwaiter<V> reader_awaiter) {
//        reader_awaiter.exec = &exec;
//        return reader_awaiter;
//    }
//
//    // for co_await channel.write()
//    // channel.write() returns an awaiter
//    template<typename V>
//    auto await_transform(WriteAwaiter<V> writer_awaiter) {
//        writer_awaiter.exec = &exec;
//        return writer_awaiter;
//    }

    T get_result() {
        std::unique_lock lock(completion_lock);
        if (!result.has_value()) {
            // not ready, wait
            completion.wait(lock);
        }
        // ready by other threads, return value
        return result->get_or_throw();
    }

    void on_completed(std::function<void(Ret<T>)> &&func) {
        std::unique_lock lock(completion_lock);
        if (result.has_value()) {
            // If result has a value, call this function callback
            auto value = result.value();
            lock.unlock();
            func(value);
        } else {
            // otherwise, add it to callback list
            completion_callbacks.push_back(func);
        }
    }
private:
    std::optional<Ret<T>> result;
    std::mutex completion_lock;
    std::condition_variable completion;
    std::list<std::function<void(Ret<T>)>> completion_callbacks;
    Exec exec;

    void notify_callbacks() {
        auto value = result.value();
        for (auto &callback : completion_callbacks) {
            callback(value);
        }
        completion_callbacks.clear();
    }
};

template<typename Exec>
struct TaskPromise<void, Exec> {
    // ~~execute the coroutine immediately~~
    // Create a dispatcher, firstly suspend, then let executor resume it
    DispatchAwaiter initial_suspend() { return DispatchAwaiter{&exec}; }

    // need to suspend to wait for the cleaning finished
    std::suspend_always final_suspend() noexcept { return {}; }

    // create the return Task<T> object
    Task<void, Exec> get_return_object() {
        return Task{std::coroutine_handle<TaskPromise>::from_promise(*this)};
    }

    // store the exception to the promise if coroutine has un-caught exception
    void unhandled_exception() {
        std::lock_guard lock(completion_lock);
        result = Ret<void>(std::current_exception());
        // finished with exception, notify all
        completion.notify_all();
        // notify callbacks
        notify_callbacks();
    }

    // for co_return, to store the return value
    void return_void() {
        std::lock_guard lock(completion_lock);
        result = Ret<void>();
        // finished, notify all
        completion.notify_all();
        // notify callbacks
        notify_callbacks();
    }

    /**
     * co_await <XXX>, where <XXX> is an awaiter.
     * Here we register the executor to the passing awaiter.
     */
    template<typename AwaiterImpl>
    requires AwaiterImplRestriction<AwaiterImpl, typename AwaiterImpl::RetType>
    AwaiterImpl await_transform(AwaiterImpl awaiter) {
        awaiter.install_executor(&exec);
        return awaiter;
    }

    template<typename U, typename _Exec>
    TaskAwaiter<U, _Exec> await_transform(Task<U, _Exec>&& task) {
        return await_transform(TaskAwaiter<U, _Exec>(std::move(task)));
    }

    template<typename _Rep, typename _Period>
    SleepAwaiter await_transform(std::chrono::duration<_Rep, _Period> &&duration) {
        return await_transform(SleepAwaiter(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));
    }

    void get_result() {
        std::unique_lock lock(completion_lock);
        if (!result.has_value()) {
            // not ready, wait
            completion.wait(lock);
        }
        // ready by other threads, return value
        result->get_or_throw();
    }

    void on_completed(std::function<void(Ret<void>)> &&func) {
        std::unique_lock lock(completion_lock);
        if (result.has_value()) {
            // If result has a value, call this function callback
            auto value = result.value();
            lock.unlock();
            func(value);
        } else {
            // otherwise, add it to callback list
            completion_callbacks.push_back(func);
        }
    }
private:
    std::optional<Ret<void>> result;
    std::mutex completion_lock;
    std::condition_variable completion;
    std::list<std::function<void(Ret<void>)>> completion_callbacks;
    Exec exec;

    void notify_callbacks() {
        auto value = result.value();
        for (auto &callback : completion_callbacks) {
            callback(value);
        }
        completion_callbacks.clear();
    }
};



#endif //COROUTINE_TASK_PROMISE_H
