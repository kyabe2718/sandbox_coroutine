#undef NDEBUG
#include <cassert>
#include <iostream>

#include <coro/sync_wait.hpp>
#include <coro/task.hpp>

#include <boost/core/typeinfo.hpp>
#include <thread>
#include <vector>

namespace coro {
struct async_mutex {

    struct lock_operation;

    static inline std::uintptr_t not_locked = 0;
    static inline std::uintptr_t no_waiters = 1;
    std::atomic<std::uintptr_t> m_state = not_locked;
    lock_operation *m_awaiters;

    struct lock_operation {

        explicit lock_operation(async_mutex &mutex)
            : m_mutex(mutex) {}

        auto await_ready() const noexcept { return false; }

        auto await_suspend(std::coroutine_handle<> handle) noexcept -> bool {
            // trueを返せば中断，falseを返せば中断しない
            // -> 他にlockを取られていればtrueを，取られていなければfalseを返す

            m_awaiter = handle;

            auto current_state = m_mutex.m_state.load(std::memory_order_acquire);
            while (true) {
                if (current_state == not_locked) {
                    if (m_mutex.m_state.compare_exchange_weak(
                                current_state,
                                no_waiters,
                                std::memory_order_acquire,// m_mutex.m_stateを書き換えるorder
                                std::memory_order_relaxed // current_stateを書き換えるorder
                                )) {
                        return false;
                    }
                } else {
                    m_next = reinterpret_cast<lock_operation *>(current_state);
                    if (m_mutex.m_state.compare_exchange_weak(
                                current_state,
                                reinterpret_cast<std::uintptr_t>(this),
                                std::memory_order_release,
                                std::memory_order_relaxed)) {
                        return true;
                    }
                }
            }
        }

        auto await_resume() const noexcept {}

    protected:
        async_mutex &m_mutex;
        lock_operation *m_next = nullptr;
        std::coroutine_handle<> m_awaiter = nullptr;
    };

    auto lock() -> lock_operation {
        return lock_operation{*this};
    }

    auto unlock() {
        auto *awaiters = m_awaiters;
        if (awaiters == nullptr) {
            auto old_state = no_waiters;
            if (m_state.compare_exchange_weak(
                        old_state,
                        not_locked,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                return;
            }
        }
    }
};
}// namespace coro

int num = 0;
coro::async_mutex mtx;

auto task() -> coro::task<> {
    co_await mtx.lock();
    co_return;
}

int main() {

    std::cout << std::endl;

    std::vector<std::thread> vec(10);// (1)

    for (auto &thr : vec) {
        thr = std::thread([] { for(int n = 0; n < 100; ++n) coro::sync_wait(task()); });
    }

    for (auto &thr : vec) thr.join();

    std::cout << "num: " << num << std::endl;

    std::cout << std::endl;
}