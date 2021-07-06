#include <coro/sync_wait.hpp>
#include <coro/task.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

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

TEST_CASE("awaiting default-constructed task throws broken_promise") {
    coro::sync_wait([&]() -> coro::task<> {
        coro::task<> t;
        CHECK_THROWS_AS(co_await t, const coro::broken_promise &);
    }());
}

TEST_CASE("task of reference type") {
    int value = 3;
    auto f = [&]() -> coro::task<int &> {
        co_return value;
    };

    coro::sync_wait([&]() -> coro::task<> {
        SUBCASE("awaiting rvalue task") {
            decltype(auto) result = co_await f();
            static_assert(
                    std::is_same<decltype(result), int &>::value,
                    "co_await r-value reference of task<int&> should result in an int&");
            CHECK(&result == &value);
        }

        SUBCASE("awaiting lvalue task") {
            auto t = f();
            decltype(auto) result = co_await t;
            static_assert(
                    std::is_same<decltype(result), int &>::value,
                    "co_await l-value reference of task<int&> should result in an int&");
            CHECK(&result == &value);
        }
    }());
}
TEST_SUITE_END();