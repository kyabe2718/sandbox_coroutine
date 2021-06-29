#pragma once

#include "coro/utility/coro_base.hpp"
#include "sync.hpp"

#include <iostream>

namespace coro {

namespace detail {
template<typename T, typename Sync /*= ConditionVariableSync*/>
struct sync_wait_task;

template<typename T, typename Sync>
struct sync_wait_promise : add_return_value_or_void<promise_base<T>, (std::is_void_v<T> ? return_type::return_void : return_type::return_value)> {

    friend struct sync_wait_task<T, Sync>;

    auto get_return_object() -> sync_wait_task<T, Sync>;

    auto initial_suspend() const noexcept {
        return std::suspend_always{};
    }

    auto final_suspend() const noexcept {
        struct completion_notifier {
            auto await_ready() const noexcept { return false; }
            void await_suspend(std::coroutine_handle<sync_wait_promise<T, Sync>> handle) const noexcept {
                handle.promise().notify();
            }
            auto await_resume() const noexcept {}
        };
        return completion_notifier{};
    }

    void notify() { sync.notify_one(); }

private:
    Sync sync;
};

template<typename T, typename Sync>
struct sync_wait_task : coroutine_base<sync_wait_promise<T, Sync>> {
    using base_type = coroutine_base<sync_wait_promise<T, Sync>>;
    using typename base_type::handle_type;
    using typename base_type::promise_type;

    using base_type::base_type;

    friend promise_type;

    void start() { return this->resume(); }

    void wait() { this->promise().sync.wait(); }

    decltype(auto) result() {
        if constexpr (std::is_void_v<T>) {
            return;
        } else {
            return this->promise().value();
        }
    }

private:
    using base_type::resume;
};

template<typename T, typename Sync>
auto sync_wait_promise<T, Sync>::get_return_object() -> sync_wait_task<T, Sync> {
    using handle_t = typename sync_wait_task<T, Sync>::handle_type;
    static_assert(Promise<sync_wait_promise<T, Sync>>);
    return sync_wait_task<T, Sync>(handle_t::from_promise(*this));
}
}// namespace detail

template<typename Sync = ConditionVariableSync, typename Awaitable>
decltype(auto) sync_wait(Awaitable &&awaitable) {

    using await_result_t = typename awaitable_traits<Awaitable>::await_result_type;

    auto wait_task = [&awaitable]() -> detail::sync_wait_task<await_result_t, Sync> {
        if constexpr (std::is_void_v<await_result_t>) {
            co_await awaitable;
        } else {
            co_return co_await awaitable;
        }
    }();

    wait_task.start();

    wait_task.wait();

    return wait_task.result();
}
}// namespace coro