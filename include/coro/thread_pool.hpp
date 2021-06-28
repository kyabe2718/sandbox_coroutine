#pragma once

#include <thread>
#include <vector>

namespace coro {

template<typename thread_t = std::thread>
struct thread_pool {

    void run_worker_thread(int idx) {
    }

private:
    std::vector<thread_t> m_threads;
};

}// namespace coro
