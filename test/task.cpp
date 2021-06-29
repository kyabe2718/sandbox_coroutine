#include <coro/task.hpp>
#include <coro/sync_wait.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

//namespace coro {
//template<typename Awaitable>
//void sync_wait(Awaitable &&awaitable) {
//    auto do_task = [&]() -> generator<int> {
//        co_yield(co_await std::forward<Awaitable>(awaitable));
//    }();
//    do_task();
//}
//}// namespace coro

#include <doctest/doctest.h>

TEST_SUITE_BEGIN("task");

TEST_CASE("task doesn't start until awaited") {
    bool started = false;
    auto func = [&]() -> coro::task<int> {
        started = true;
        co_return 1;
    };

    sync_wait([&]() -> coro::task<int> {
        auto t = func();
        CHECK(!started);

        co_await t;

        CHECK(started);
        co_return 1;
    }());
}

TEST_SUITE_END();


//int main() {
//    static_assert(coro::Awaitable<coro::task<void>>);
//
//    auto func = []() -> coro::task<int> {
//        std::cout << "func" << std::endl;
//        std::this_thread::sleep_for(1s);
//        co_return 10;
//    };
//
//    auto task = [&]() -> coro::task<int> {
//        auto t = func();
//
//        std::cout << "start" << std::endl;
//        int v = co_await t;
//        std::cout << v << std::endl;
//        std::cout << "end " << std::endl;
//
//        co_return 10;
//    };
//
//    // task().resume();
//
//    auto do_task = [&]() -> coro::generator<int> {
//        co_yield co_await task();
//    }();
//    do_task();
//}
