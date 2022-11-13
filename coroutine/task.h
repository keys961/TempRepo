//
// Created by Jintao Ye on 10/11/22.
//

#ifndef COROUTINE_TASK_H
#define COROUTINE_TASK_H

#include <coroutine>
#include <type_traits>
#include <iostream>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <list>
#include "executor.h"
#include "timer.h"
#include "channel.h"
#include "ret.h"
#include "task_promise.h"


template<typename T, typename Exec>
struct TaskAwaiter;

template<typename T, typename Exec = NoopExecutor>
struct Task {
    using promise_type = TaskPromise<T, Exec>;

    std::coroutine_handle<promise_type> handle;

    explicit Task(std::coroutine_handle<promise_type> handle) noexcept : handle{handle} {}
    Task(Task &&task) noexcept : handle{std::exchange(task.handle, {})} {}
    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;
    ~Task() {
        if (handle) {
            handle.destroy();
        }
    }

    T get_result() {
        // this will be blocked until the Task coroutine is finished
        return handle.promise().get_result();
    }

    Task &then(std::function<void(T)> &&func) {
        handle.promise().on_completed([func](auto result) {
            try {
                func(result.get_or_throw());
            } catch (std::exception &e) {
                // 忽略异常
            }
        });
        return *this;
    }

    Task &catching(std::function<void(std::exception &)> &&func) {
        handle.promise().on_completed([func](auto result) {
            try {
                // 忽略返回值
                result.get_or_throw();
            } catch (std::exception &e) {
                func(e);
            }
        });
        return *this;
    }

    Task &finally(std::function<void()> &&func) {
        handle.promise().on_completed([func](auto result) { func(); });
        return *this;
    }

    auto as_awaiter() {
        return TaskAwaiter<T, Exec>(std::move(*this));
    }
};

template<typename Exec>
struct Task<void, Exec> {
    using promise_type = TaskPromise<void, Exec>;

    std::coroutine_handle<promise_type> handle;

    explicit Task(std::coroutine_handle<promise_type> handle) noexcept : handle{handle} {}
    Task(Task &&task) noexcept : handle{std::exchange(task.handle, {})} {}
    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;
    ~Task() {
        if (handle) {
            handle.destroy();
        }
    }

    void get_result() {
        // this will be blocked until the Task coroutine is finished
        handle.promise().get_result();
    }
};



#endif //COROUTINE_TASK_H
