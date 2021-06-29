#pragma once

#include "utility/coro_base.hpp"

#include <thread>
#include <vector>

namespace coro {

template<typename thread_t = std::thread>
struct thread_pool {

    struct schedule_awaitable {
        auto await_ready() noexcept { return false; }
        void await_suspend(std::coroutine_handle<> handle);
        auto await_resume() {}

    private:
        std::coroutine_handle<> awaiting_coroutine;
    };

    auto schedule() noexcept {
    }

private:
    std::vector<thread_t> m_threads;
};

}// namespace coro
