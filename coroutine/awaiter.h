//
// Created by Jintao Ye on 13/11/22.
//

#ifndef COROUTINE_AWAITER_H
#define COROUTINE_AWAITER_H

#include <functional>
#include <coroutine>
#include <optional>

#include "executor.h"
#include "ret.h"

template<typename T>
struct Awaiter {

    using RetType = T;

    void install_executor(AbstractExecutor* e) {
        exec = e;
    }
    /**
     * Need suspend current coroutine first or not, when calling co_await <XXX>,
     * where <XXX> returns this Awaiter.
     */
    bool await_ready() const { return false; }
    /**
     * If await_ready() returns false, current coroutine should be suspended.
     * And this function will be called.
     */
    void await_suspend(std::coroutine_handle<> h) {
        handle = h;
        after_suspend();
    }
    /**
     * If current coroutine is resumed by calling handle::resume(),
     * this function will be called.
     *
     * And it will returns a value for co_await <XXX>.
     * (i.e. T v = co_await <XXX>, here we return the v).
     */
    T await_resume() {
        before_resume();
        return ret->get_or_throw();
    }

    /**
     * Resume current coroutine with value
     */
    void resume(T v) {
        dispatch([this, v]() {
            ret = Ret<T>(static_cast<T>(v));
            handle.resume();
        });
    }

    /**
     * Resume current coroutine without value
     */
    void resume_unsafe() {
        dispatch([this]() {
            handle.resume();
        });
    }

    /**
     * Resume current coroutine with exception
     */
    void resume_exception(std::exception_ptr&& e) {
        dispatch([this, e]() {
            ret = Ret<T>(static_cast<std::exception_ptr>(e));
            handle.resume();
        });
    }
protected:
    std::optional<Ret<T>> ret;
    /**
    * Some action that after current coroutine is suspended (when calling co_await <XXX>)
    */
    virtual void after_suspend() {}

    /**
     * Some action that after current coroutine is resumed and before returning the value for co_await
     */
    virtual void before_resume() {}

private:
    AbstractExecutor* exec = nullptr;
    std::coroutine_handle<> handle;

    /**
     * Dispatch a function to an executor to execute.
     * e.g. resume a coroutine (coroutine will be scheduled on the thread in executor)
     */
    void dispatch(std::function<void()> &&f) {
        if (exec) {
            exec->execute(std::move(f));
        } else {
            f();
        }
    }
};

template<>
struct Awaiter<void> {

    using RetType = void;

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        handle = h;
        after_suspend();
    }

    void await_resume() {
        before_resume();
        ret->get_or_throw();
    }

    void resume() {
        dispatch([this]() {
            ret = Ret<void>();
            handle.resume();
        });
    }

    void resume_exception(std::exception_ptr &&e) {
        dispatch([this, e]() {
            ret = Ret<void>(static_cast<std::exception_ptr>(e));
            handle.resume();
        });
    }

    void install_executor(AbstractExecutor *executor) {
        exec = executor;
    }

    virtual void after_suspend() {}

    virtual void before_resume() {}

private:
    std::optional<Ret<void>> ret{};
    AbstractExecutor *exec = nullptr;
    std::coroutine_handle<> handle;

    void dispatch(std::function<void()> &&f) {
        if (exec) {
            exec->execute(std::move(f));
        } else {
            f();
        }
    }
};
#endif //COROUTINE_AWAITER_H
