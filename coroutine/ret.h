//
// Created by Jintao Ye on 13/11/22.
//

#ifndef COROUTINE_RET_H
#define COROUTINE_RET_H
template<typename T>
struct Ret {
private:
    T value{};
    std::exception_ptr e_ptr;
public:
    explicit Ret() = default;
    explicit Ret(T&& v) : value{std::forward<T>(v)} {}
    explicit Ret(std::exception_ptr&& ptr) : e_ptr{ptr} {}

    T get_or_throw() {
        if (e_ptr) {
            std::rethrow_exception(e_ptr);
        }
        return value;
    }
};

template<>
struct Ret<void> {
private:
    std::exception_ptr e_ptr;
public:
    explicit Ret() = default;
    explicit Ret(std::exception_ptr&& ptr) : e_ptr{ptr} {}

    void get_or_throw() {
        if (e_ptr) {
            std::rethrow_exception(e_ptr);
        }
    }
};
#endif //COROUTINE_RET_H
