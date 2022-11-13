//
// Created by Jintao Ye on 13/11/22.
//

#ifndef COROUTINE_SLEEP_AWAITER_H
#define COROUTINE_SLEEP_AWAITER_H

#include <coroutine>
#include "awaiter.h"
#include "timer.h"

struct SleepAwaiter : public Awaiter<void> {
    explicit SleepAwaiter(long long duration) noexcept
            : duration{duration} {}

    template<typename _Rep, typename _Period>
    explicit SleepAwaiter(std::chrono::duration<_Rep, _Period> &&duration) noexcept
            : duration(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) {}

    void after_suspend() override {
        static Timer timer;
        timer.execute([this] { resume(); }, duration);
    }
private:
    long long duration;
};

#endif //COROUTINE_SLEEP_AWAITER_H
