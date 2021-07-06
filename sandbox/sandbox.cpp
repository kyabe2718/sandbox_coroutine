#include <iostream>

#include <coro/sync_wait.hpp>
#include <coro/task.hpp>

#include <thread>

using namespace std::chrono_literals;

struct thread_pool {
};


int main() {
    static_assert(coro::Awaitable<coro::task<void>>);
    static_assert(std::atomic<std::coroutine_handle<>>::is_always_lock_free);
    return 0;
}
