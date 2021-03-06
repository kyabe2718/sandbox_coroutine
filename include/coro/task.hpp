#pragma once

#include "utility/coro_base.hpp"
#include "utility/exception.hpp"

#include <cassert>
#include <iostream>

namespace coro {

/*
 * task型のコルーチン
 */

template<typename T = void>
struct task;

namespace detail {

// taskの最終サスペンドポイントで返されるべきawaitable
// await_suspendでnextを返すことで後続のcoroutineを再開する
struct final_awaitable {

    auto await_ready() const noexcept { return false; }

    template<typename P>
    auto await_suspend(std::coroutine_handle<P> handle) const noexcept {
        auto next = handle.promise().next;
        assert(next && "next handle is null");
        return next;
    }

    auto await_resume() const noexcept {}
};

template<typename T>
struct task_promise_base : add_return_value_or_void<promise_base<T>, (std::is_void_v<T> ? return_type::return_void : return_type::return_value)> {
    auto get_return_object() -> task<T>;

    auto initial_suspend() const noexcept { return std::suspend_always{}; }

    auto final_suspend() const noexcept { return final_awaitable{}; }

    void set_next(std::coroutine_handle<> next_) { next = next_; }

private:
    friend struct final_awaitable;
    std::coroutine_handle<> next;
};

template<typename T>
struct task_promise : task_promise_base<T> {

    decltype(auto) result() & {
        this->rethrow_if_exception();
        return this->value();
    }

    decltype(auto) result() && {
        this->rethrow_if_exception();
        return std::move(*this).value();
    }

    using task_promise_base<T>::task_promise_base;
};

template<>
struct task_promise<void> : task_promise_base<void> {
    void result() { this->rethrow_if_exception(); }
    using task_promise_base<void>::task_promise_base;
};
}// namespace detail

template<typename T>
struct task : coroutine_base<detail::task_promise<T>> {

    using base_type = coroutine_base<detail::task_promise<T>>;
    using promise_type = typename base_type::promise_type;
    using handle_type = typename base_type::handle_type;
    using base_type::base_type;

    // co_await t; とした時に使われるawaitable
    // m_coroutine は t に対応するコルーチンハンドル
    // m_coroutine の next に現在のコルーチンを格納し、代わりに m_coroutine を再開する
    struct awaitable_base {
        explicit awaitable_base(std::coroutine_handle<promise_type> handle) noexcept
            : m_coroutine(handle) {}

        auto await_ready() { return !m_coroutine || m_coroutine.done(); }

        auto await_suspend(std::coroutine_handle<> coro) {
            // m_coroutineが終了したら再び今のcoroutineを再開してほしい
            m_coroutine.promise().set_next(coro);
            return m_coroutine;
        }

    protected:
        // 中断された時に代わりに再開されるcoroutine
        std::coroutine_handle<promise_type> m_coroutine;
    };

    auto operator co_await() const & {
        struct awaitable : awaitable_base {
            using awaitable_base::awaitable_base;
            decltype(auto) await_resume() {
                if (!this->m_coroutine) { throw broken_promise{}; }
                return this->m_coroutine.promise().result();
            }
        };
        return awaitable{this->handle};
    }

    auto operator co_await() const && {
        struct awaitable : awaitable_base {
            using awaitable_base::awaitable_base;
            decltype(auto) await_resume() {
                if (!this->m_coroutine) { throw broken_promise{}; }
                return std::move(this->m_coroutine.promise()).result();
            }
        };
        return awaitable{this->handle};
    }

    friend struct detail::task_promise_base<T>;
};

namespace detail {
template<typename T>
auto task_promise_base<T>::get_return_object() -> task<T> {
    static_assert(Promise<detail::task_promise<T>>);
    using handle_type = typename task<T>::handle_type;
    return task<T>(handle_type::from_promise(static_cast<task_promise<T> &>(*this)));
}
}// namespace detail
}// namespace coro
