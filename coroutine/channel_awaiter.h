//
// Created by Jintao Ye on 13/11/22.
//

#ifndef COROUTINE_CHANNEL_AWAITER_H
#define COROUTINE_CHANNEL_AWAITER_H

#include <coroutine>
#include "executor.h"
#include "awaiter.h"

template<typename V>
struct Channel;

template<typename V>
struct WriteAwaiter : public Awaiter<void> {
    Channel<V> *chan;
    V val;

    WriteAwaiter(Channel<V> *chan, V val) : Awaiter<void>(), chan{chan}, val{val} {}
    WriteAwaiter(WriteAwaiter&& other) noexcept :
            Awaiter(other),
            chan{std::exchange(other.chan, nullptr)},
            val{other.val}
    {}

    void after_suspend() override {
        // just try to write
        // if not OK, suspended
        // or will be resumed in try_push_writer
        chan->try_push_writer(this);
    }

    void before_resume() override {
        chan->check_closed();
        chan = nullptr;
    }

    ~WriteAwaiter() {
        if (chan) {
            chan->remove_writer(this);
        }
    }
};

template<typename V>
struct ReadAwaiter : public Awaiter<V> {
    Channel<V> *chan;
    // outside value address to receive value address
    V *p_value = nullptr;

    ReadAwaiter(Channel<V> *chan) : Awaiter<V>(), chan{chan} {}
    ReadAwaiter(ReadAwaiter&& other) noexcept :
            Awaiter<V>(other),
            chan{std::exchange(other.chan, nullptr)},
            p_value{std::exchange(other.p_value, nullptr)}
    {}

    void after_suspend() override {
        // just try to read
        // if not OK, suspended
        // or will be resumed in try_push_reader
        chan->try_push_reader(this);
    }

    void before_resume() override {
        chan->check_closed();
        if (p_value) {
            // fetch value from channel if finished
            *p_value = this->ret->get_or_throw();
        }
        chan = nullptr;
    }

    ~ReadAwaiter() {
        if (chan) {
            chan->remove_reader(this);
        }
    }
};
#endif //COROUTINE_CHANNEL_AWAITER_H
