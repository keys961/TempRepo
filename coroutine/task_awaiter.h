//
// Created by Jintao Ye on 13/11/22.
//

#ifndef COROUTINE_TASK_AWAITER_H
#define COROUTINE_TASK_AWAITER_H

#include <coroutine>

template<typename T, typename Exec>
struct Task;

// co_await Task<U> -> return TaskAwaiter
// Then execute await_ready() -> not suspended current coroutine (not Task<U>)
// Then execute await_suspend() -> register a resume callback when Task<U> finished,
// and suspend current coroutine
// (not Task<U>, Task<U> will being executed at once since its promise_type returns std::suspend_never in initial_suspend)

// Core of coroutine schedule
template<typename T, typename Exec>
struct TaskAwaiter : public Awaiter<T> {
private:
    Task<T, Exec> task;
public:
    explicit TaskAwaiter(Task<T, Exec> &&task) noexcept
            : Awaiter<T>(), task{std::move(task)} {}
    TaskAwaiter(TaskAwaiter&& other) noexcept
        : Awaiter<T>(other), task{std::move(other.task)} {}

    TaskAwaiter(TaskAwaiter &) = delete;
    TaskAwaiter &operator=(TaskAwaiter &) = delete;

    void after_suspend() override {
        task.finally([this]() {
            // 先不去获取结果，原因是除了正常的返回值以外，还可能是异常
            this->resume_unsafe();
        });
    }

    void before_resume() override {
        // 如果有返回值，则赋值给 ret，否则直接抛异常
        this->ret = Ret<T>(task.get_result());
    }

    auto as_awaiter() {
        return TaskAwaiter<T, Exec>(std::move(*this));
    }
};

#endif //COROUTINE_TASK_AWAITER_H
