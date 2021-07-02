#undef NDEBUG
#include <cassert>
#include <iostream>

#include <coro/sync_wait.hpp>
#include <coro/task.hpp>

#include <queue>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

struct worker_thread {
    struct sched_op {
        auto await_ready() noexcept { return false; }
        auto await_suspend(std::coroutine_handle<> awaiting) noexcept {
            m_worker->schedule_impl(awaiting);
            return m_worker->next_task();
        }

    private:
        worker_thread *m_worker;
    };

    void schedule_impl(std::coroutine_handle<> coro) {
    }

    std::coroutine_handle<> next_task() {
    }
};

int main() {
    static_assert(coro::Awaitable<coro::task<void>>);

    thread_pool th;

    auto task = [&]() -> coro::task<void> {
        std::cout << "hoge" << std::endl;

        co_await th.schedule();

        std::cout << "fuga" << std::endl;

        co_return;
    };

    auto t = task();

    th.run();

    coro::sync_wait<coro::ConditionVariableSync>(t);
}
