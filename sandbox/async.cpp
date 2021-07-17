#undef NDEBUG
#include <cassert>

#include <coro/generator.hpp>
#include <coro/sync_wait.hpp>
#include <coro/task.hpp>

#include <iostream>
#include <thread>
#include <vector>

#include <boost/core/typeinfo.hpp>
#include <boost/lockfree/queue.hpp>

namespace coro {

template<typename T = void>
struct async;

namespace detail {
template<typename T>
struct async_promise
    : add_return_value_or_void<promise_base<T>, value_or_void<T>> {

    auto get_return_object() -> async<T>;

    auto initial_suspend() noexcept {
        return std::suspend_always{};
    }

    auto final_suspend() noexcept {
        return std::suspend_always{};
    }
};
}// namespace detail

template<typename T>
struct async : coroutine_base<typename detail::async_promise<T>> {
    using promise_type = typename detail::async_promise<T>;
    using base_type = coroutine_base<promise_type>;
    using handle_type = typename base_type::handle_type;

    friend promise_type;
    using base_type::base_type;

    auto await_ready() {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        // ここでmoveしてランタイム行き？
        return true;
    }

    auto await_suspend(std::coroutine_handle<>) {
        std::cout << "unreachable" << std::endl;
        std::terminate();
    }

    auto await_resume() {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};

template<typename T>
auto detail::async_promise<T>::get_return_object() -> async<T> {
    using handle_type = typename async<T>::handle_type;
    return async<T>(handle_type::from_promise(*this));
}

}// namespace coro

using namespace std::chrono_literals;


int main() {

    auto async_read = []() -> coro::task<std::string> {
        std::this_thread::sleep_for(1s);
        co_return "message";
    };

    auto func = [&]() -> coro::task<> {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << (co_await async_read()) << std::endl;
    };

    coro::sync_wait(func());
}