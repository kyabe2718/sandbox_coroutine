
#include <coro/task.hpp>
#include <coro/generator.hpp>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

int main() {
    static_assert(coro::Awaitable<coro::task<void>>);

    auto func = []() -> coro::task<int> {
        std::cout << "func" << std::endl;
        std::this_thread::sleep_for(1s);
        co_return 10;
    };

    auto task = [&]() -> coro::task<int> {
        auto t = func();

        std::cout << "start" << std::endl;
        int v = co_await t;
        std::cout << v << std::endl;
        std::cout << "end " << std::endl;

        co_return 10;
    };

    task().resume();

//    auto do_task = [&]() -> coro::generator<int> {
//        co_yield co_await task();
//    }();
//    do_task.move_next();
}
