#pragma once

#include "coro_base.hpp"
#include <iostream>

namespace coro {

/*
 * task型のコルーチン
 */

template<typename T = void>
struct task;

namespace detail {

struct final_awaitable {
    auto await_ready() const noexcept { return false; }

    template<typename P>
    auto await_suspend(std::coroutine_handle<P> handle) const noexcept {
        auto next = handle.promise().next;
        //  if (!next || next.done())
        //      throw std::runtime_error{"try resume null coroutine or finished coroutine"}; // await_suspend cannot throw
        return next;
    }

    auto await_resume() const noexcept {}
};

template<typename T>
struct task_promise : promise_base<T> {

    auto get_return_object() -> task<T>;

    auto initial_suspend() const noexcept { return std::suspend_always{}; }
    auto final_suspend() const noexcept { return final_awaitable{}; }

    template<typename U>
    void return_value(U &&v) {
        this->template set_value(std::forward<U>(v));
    }

    decltype(auto) result() & {
        this->rethrow_if_exception();
        return this->value();
    }

    decltype(auto) result() && {
        this->rethrow_if_exception();
        return std::move(*this).value();
    }

    void set_next(std::coroutine_handle<> next_) { next = next_; }

private:
    friend struct final_awaitable;
    std::coroutine_handle<> next;
};

template<>
struct task_promise<void> : promise_base<void> {

    auto get_return_object() -> task<void>;

    auto initial_suspend() const noexcept { return std::suspend_always{}; }
    auto final_suspend() const noexcept { return final_awaitable{}; }

    void return_void() {}

    decltype(auto) result() { this->rethrow_if_exception(); }

    void set_next(std::coroutine_handle<> next_) { next = next_; }

private:
    friend struct final_awaitable;
    std::coroutine_handle<> next;
};
}// namespace detail

template<typename T>
struct task : coroutine_base<detail::task_promise<T>> {

    using base_type = coroutine_base<detail::task_promise<T>>;
    using promise_type = typename base_type::promise_type;
    using handle_type = typename base_type::handle_type;
    using base_type::base_type;

    struct awaitable_base {
        explicit awaitable_base(std::coroutine_handle<promise_type> handle) noexcept
            : m_coroutine(handle) {}

        auto await_ready() { return !m_coroutine || m_coroutine.done(); }

        template<typename P>
        auto await_suspend(std::coroutine_handle<P> coro) {
            m_coroutine.promise().set_next(coro);
            return m_coroutine;
        }

    protected:
        std::coroutine_handle<promise_type> m_coroutine;
    };


    auto operator co_await() const & {
        struct awaitable : awaitable_base {
            using awaitable_base::awaitable_base;
            decltype(auto) await_resume() { return this->m_coroutine.promise().result(); }
        };
        return awaitable{this->handle};
    }

    auto operator co_await() const && {
        struct awaitable : awaitable_base {
            using awaitable_base::awaitable_base;
            decltype(auto) await_resume() { return std::move(this->m_coroutine.promise()).result(); }
        };
        return awaitable{this->handle};
    }

    friend promise_type;
};

namespace detail {
template<typename T>
auto task_promise<T>::get_return_object() -> task<T> {
    static_assert(Promise<detail::task_promise<T>>);
    using handle_type = typename task<T>::handle_type;
    return task<T>(handle_type::from_promise(*this));
}

inline auto task_promise<void>::get_return_object() -> task<void> {
    static_assert(Promise<detail::task_promise<void>>);
    using handle_type = typename task<void>::handle_type;
    return task<void>(handle_type::from_promise(*this));
}
}// namespace detail

//static_assert(Awaiter<awaitable_to_awaiter_t<task<void>>>);
}// namespace coro
