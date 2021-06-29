#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace coro {

struct ConditionVariableSync {

    ConditionVariableSync()
        : flag(true) {}

    void reset() {
        std::unique_lock lk(mtx);
        flag = true;
    }

    void wait() {
        std::unique_lock lk(mtx);
        cv.wait(lk, [this] { return !flag; });
    }

    void notify_one() {
        std::unique_lock lk(mtx);
        flag = false;
        cv.notify_one();
    }

    void notify_all() {
        std::unique_lock lk(mtx);
        flag = false;
        cv.notify_all();
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    bool flag;
};

struct BusyLoopSync {

    BusyLoopSync() {
        flag.test_and_set(std::memory_order_release);
    }

    void reset() {
        flag.test_and_set(std::memory_order_release);
    }

    void wait() {
        while (flag.test_and_set(std::memory_order_acquire))
            ;
    }

    void notify_one() {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag;
};

}// namespace coro