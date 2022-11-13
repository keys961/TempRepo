//
// Created by Jintao Ye on 13/11/22.
//

#ifndef COROUTINE_FUTURE_AWAITER_H
#define COROUTINE_FUTURE_AWAITER_H
#include <future>
#include "awaiter.h"

template<typename R>
struct FutureAwaiter : public Awaiter<R> {
    explicit FutureAwaiter(std::future<R> &&future) noexcept
            : Awaiter<R>(), fut{std::move(future)} {}

    FutureAwaiter(FutureAwaiter &&awaiter) noexcept
            : Awaiter<R>(awaiter), fut{std::move(awaiter.fut)} {}

    FutureAwaiter(FutureAwaiter &) = delete;

    FutureAwaiter &operator=(FutureAwaiter &) = delete;

protected:
    void after_suspend() override {
        // std::future::get 会阻塞等待结果的返回，因此我们新起一个线程等待结果的返回
        // 如果后续 std::future 增加了回调，这里直接注册回调即可
        std::thread([this](){
            // 获取结果，并恢复协程
            this->resume(this->fut.get());
        }).detach();
        // std::thread 必须 detach 或者 join 二选一
        // 也可以使用 std::jthread
    }

private:
    std::future<R> fut;
};

template<typename R>
FutureAwaiter<R> as_awaiter(std::future<R> &&future) {
    return FutureAwaiter(std::move(future));
}

#endif //COROUTINE_FUTURE_AWAITER_H
