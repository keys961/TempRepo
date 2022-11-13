#ifndef ENTRY_TASK_THREADPOOL_H
#define ENTRY_TASK_THREADPOOL_H

#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <stdexcept>
#include <functional>

#define THREAD_POOL_MAX_NUM 16

class ThreadPool {
    using Task = std::function<void()>;
    std::vector<std::thread> pool;     // pool thread
    std::queue<Task> tasks;            // task queue
    std::mutex mu;                   // for sync
    std::condition_variable task_cv;   // cond
    std::atomic<bool> run{true };     // pool is running or not
    std::atomic<int>  idle{0 };  // idle thread count

public:
    inline ThreadPool(unsigned short size = 4) { addThread(size); }
    inline ~ThreadPool() {
        run = false;
        task_cv.notify_all(); // wake up all the thread
        for (std::thread& thread : pool) {
            if(thread.joinable()) {
                thread.join(); // wait task finished
            }
        }
    }

    public:
        // submit a task:
        // 1. just call a func
        // 2. bind： .commit(std::bind(&Dog::sayHello, &dog));
        // 3. mem_fn： .commit(std::mem_fn(&Dog::sayHello), this)
        template<class F, class... Args>
        auto submit(F&& f, Args&&... args) ->std::future<decltype(f(args...))> {
            if (!run) {
                throw std::runtime_error("submit on ThreadPool is stopped.");
            }
            using RetType = decltype(f(args...)); // typename std::result_of<F(Args...)>::type
            auto task = std::make_shared<std::packaged_task<RetType()>>(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
            std::future<RetType> future = task->get_future();
            {
                std::lock_guard<std::mutex> lock{mu };
                tasks.emplace([task](){
                    (*task)();
                });
            }
            task_cv.notify_one(); // wake up one thread
            return future;
        }

        int idleCount() { return idle; }
        int threadCount() { return pool.size(); }
    private:
        void addThread(unsigned short size) {
            for (; pool.size() < THREAD_POOL_MAX_NUM && size > 0; --size) {
                pool.emplace_back([this]{
                    while (run) {
                        Task task;
                        {
                            std::unique_lock<std::mutex> lock{mu };
                            task_cv.wait(lock, [this]{
                                return !run || !tasks.empty();
                            }); // wait for task
                            if (!run && tasks.empty())
                                return;
                            task = move(tasks.front());
                            tasks.pop();
                        }
                        idle--;
                        task();//execute
                        idle++;
                    }
                });
                idle++;
            }
        }
    };
#endif //ENTRY_TASK_THREADPOOL_H
