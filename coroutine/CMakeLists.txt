cmake_minimum_required(VERSION 3.22)
project(coroutine)

set(CMAKE_CXX_STANDARD 20)

add_executable(coroutine main.cpp generator.h task.h ioutils.h ioutils.cpp executor.h timer.h channel.h channel_awaiter.h awaiter.h sleep_awaiter.h ret.h task_awaiter.h future_awaiter.h task_promise.h)

target_link_libraries(coroutine cpp-httplib)