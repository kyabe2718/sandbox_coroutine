#include <iostream>

#include <coro/sync_wait.hpp>
#include <coro/task.hpp>

#include <queue>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

struct thread_pool {
    struct awaitable {
        explicit awaitable(thread_pool *thread_pool)
            : m_thread_pool(thread_pool) {}

        auto await_ready() { return false; }

        void await_suspend(std::coroutine_handle<> h) {
            std::unique_lock lk(m_thread_pool->mtx);
            m_thread_pool->task_queue.push(h);
            m_thread_pool->cv.notify_one();
        }

        auto await_resume() {}

    private:
        thread_pool *m_thread_pool;
    };

    auto schedule() noexcept {
        return awaitable{this};
    }

    void quit() {
        quit_flag.store(true, std::memory_order_release);
    }

    void run() {
        while (!quit_flag.load(std::memory_order_release)) {
            std::unique_lock lk(mtx);
            cv.wait(lk, [this] {
                return !task_queue.empty();
            });
            auto task = task_queue.front();
            task.resume();
        }
    }

private:
    std::vector<std::thread> m_threads;

    std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::coroutine_handle<>> task_queue;

    std::atomic<bool> quit_flag;
};

int main() {
    static_assert(coro::Awaitable<coro::task<void>>);

    thread_pool th;

    std::thread th1{[&] {
        th.run();
    }};

    auto task = [&]() -> coro::task<void> {
        std::cout << "hoge" << std::endl;

        co_await th.schedule();

        std::cout << "hoge" << std::endl;

        co_return;
    };

    coro::sync_wait<coro::ConditionVariableSync>(task());

    th.quit();
    th1.join();
}
