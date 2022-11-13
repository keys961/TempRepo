//
// Created by Jintao Ye on 13/11/22.
//

#ifndef COROUTINE_CHANNEL_H
#define COROUTINE_CHANNEL_H
#include <exception>
#include <atomic>
#include <list>
#include <mutex>
#include <coroutine>
#include "executor.h"
#include "channel_awaiter.h"

template<typename V>
struct Channel {

    struct ChannelClosedException : std::exception {
        const char *what() const noexcept override {
            return "Channel is closed.";
        }
    };

    void check_closed() {
        // throw error when it is closed
        if (!active.load(std::memory_order_relaxed)) {
            throw ChannelClosedException();
        }
    }

    explicit Channel(int capacity = 0) : buffer_capacity(capacity) {
        active.store(true, std::memory_order_relaxed);
    }

    // true 表示 Channel 尚未关闭
    bool is_active() {
        return active.load(std::memory_order_relaxed);
    }

    // 关闭 Channel
    void close() {
        bool expect = true;
        // 判断如果已经关闭，则不再重复操作
        // 比较 active 为 true 时才会完成设置操作，并且返回 true
        if(active.compare_exchange_strong(expect, false, std::memory_order_relaxed)) {
            // 清理资源
            clean_up();
        }
    }

    Channel(Channel &&channel) = delete;
    Channel(Channel &) = delete;
    Channel &operator=(Channel &) = delete;
    ~Channel() { close(); }

    auto write(V value) {
        check_closed();
        return WriteAwaiter<V>(this, value);
    }
    auto operator<<(V value) {
        return write(value);
    }

    auto read() {
        check_closed();
        return ReadAwaiter<V>(this);
    }
    auto operator>>(V& ref) {
        auto awaiter = read();
        awaiter.p_value = &ref;
        return awaiter;
    }

    void try_push_writer(WriteAwaiter<V> *writer_awaiter) {
        std::unique_lock lock(channel_lock);
        check_closed();
        // check suspended reader
        // if so, resume it with value
        if (!reader_list.empty()) {
            auto reader = reader_list.front();
            reader_list.pop_front();
            lock.unlock();

            reader->resume(writer_awaiter->val);
            writer_awaiter->resume();
            return;
        }

        // if buffer not full, just resume it
        if (buffer.size() < buffer_capacity) {
            buffer.push(writer_awaiter->val);
            lock.unlock();
            writer_awaiter->resume();
            return;
        }

        // if buffer is buff, push it to writer list
        // writer will be suspended by its caller await_suspend()
        writer_list.push_back(writer_awaiter);
    }

    void try_push_reader(ReadAwaiter<V> *reader_awaiter) {
        std::unique_lock lock(channel_lock);
        check_closed();

        // if buffer not empty
        // jut fetch value and wake up writer
        if (!buffer.empty()) {
            auto value = buffer.front();
            buffer.pop();

            if (!writer_list.empty()) {
                auto writer = writer_list.front();
                writer_list.pop_front();
                buffer.push(writer->val);
                lock.unlock();

                writer->resume();
            } else {
                lock.unlock();
            }

            reader_awaiter->resume(value);
            return;
        }

        // if there is a suspended writer
        // fetch one writer and wake it up
        if (!writer_list.empty()) {
            auto writer = writer_list.front();
            writer_list.pop_front();
            lock.unlock();

            reader_awaiter->resume(writer->val);
            writer->resume();
            return;
        }

        // finally, buffer empty,
        // reader will be suspended by its caller await_suspend()
        reader_list.push_back(reader_awaiter);
    }

    void remove_reader(ReadAwaiter<V> *reader_awaiter) {
        std::lock_guard lock(channel_lock);
        reader_list.remove(reader_awaiter);
    }

    void remove_writer(WriteAwaiter<V> *writer_awaiter) {
        std::lock_guard lock(channel_lock);
        writer_list.remove(writer_awaiter);
    }

private:
    int buffer_capacity;
    std::queue<V> buffer;
    // buffer full for writer
    std::list<WriteAwaiter<V>*> writer_list;
    // buffer empty for reader
    std::list<ReadAwaiter<V>*> reader_list;
    // Channel 的状态标识
    std::atomic<bool> active;

    std::mutex channel_lock;
    std::condition_variable channel_condition;

    void clean_up() {
        std::lock_guard lock(channel_lock);
        for (auto writer : writer_list) {
            writer->resume();
        }
        writer_list.clear();
        for (auto reader : reader_list) {
            reader->resume_unsafe();
        }
        reader_list.clear();
        decltype(buffer) empty_buffer;
        std::swap(buffer, empty_buffer);
    }
};

#endif //COROUTINE_CHANNEL_H
