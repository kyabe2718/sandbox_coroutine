#pragma once

#include "concepts.hpp"
#include <exception>

namespace coro {

template<typename P>
struct coroutine_base {
    using promise_type = P;
    using handle_type = std::coroutine_handle<promise_type>;

    coroutine_base(const coroutine_base &) = delete;
    coroutine_base &operator=(const coroutine_base &) = delete;

    ~coroutine_base() {
        if (handle) {
            handle.destroy();
        }
    }

    void resume() {
        if (handle && !handle.done()) {
            handle.resume();
        }
    }

    promise_type &promise() { return handle.promise(); }
    const promise_type &promise() const { return handle.promise(); }

    [[nodiscard]] bool done() const noexcept { return !handle or handle.done(); }

    explicit operator bool() const { return !done(); }

protected:
    coroutine_base() noexcept
        : handle(nullptr) {}

    coroutine_base(coroutine_base &&other) noexcept
        : handle(other.handle) { other.handle = nullptr; }

    explicit coroutine_base(handle_type handle) noexcept
        : handle(handle) {}

    handle_type handle;
};

template<typename T>
struct promise_base;

template<>
struct promise_base<void> {

    promise_base() = default;
    promise_base(const promise_base &) = delete;
    promise_base &operator=(const promise_base &) = delete;

    void unhandled_exception() { exception = std::current_exception(); }

    void rethrow_if_exception() {
        if (exception) {
            std::rethrow_exception(exception);
        }
    }

private:
    std::exception_ptr exception;
};

template<typename T>
struct promise_base : promise_base<void> {

    using value_type = std::remove_reference_t<T>;
    static_assert(std::is_default_constructible_v<value_type>);

    using promise_base<void>::promise_base;

    value_type &value() &noexcept { return data; }
    const value_type &value() const &noexcept { return data; }
    value_type &&value() &&noexcept { return std::move(data); }
    const value_type &&value() const &&noexcept { return std::move(data); }

    template<typename U>
    void set_value(U &&value) { data = std::forward<U>(value); }

private:
    value_type data;
};

template<typename T>
struct promise_base<T &> : promise_base<void> {
    using value_type = T;
    using reference_type = T &;

    using promise_base<void>::promise_base;

    reference_type value() noexcept { return *data; }
    std::add_const_t<reference_type> value() const noexcept { return *data; }

    template<typename U>
    void set_value(U &value) { data = std::addressof(value); }

private:
    std::add_pointer_t<value_type> data;
};

enum class return_type {
    return_value,
    return_void,
};

template<typename T>
constexpr return_type value_or_void =
        std::is_void_v<T> ? return_type::return_void
                          : return_type::return_value;

template<typename T, return_type>
struct add_return_value_or_void;

template<typename T>
struct add_return_value_or_void<T, return_type::return_value> : T {
    template<typename U>
    void return_value(U &&v) { this->template set_value(std::forward<U>(v)); }
};

template<typename T>
struct add_return_value_or_void<T, return_type::return_void> : T {
    void return_void() {}
};

struct coroutine_iterator_end {};

template<typename C>
struct coroutine_iterator {
    using coroutine_type = C;

    coroutine_iterator() noexcept : coro(nullptr) {}

    explicit coroutine_iterator(coroutine_type &coro) noexcept
        : coro(std::addressof(coro)) {}

    coroutine_iterator &operator++() {
        coro->move_next();
        return *this;
    }

    void operator++(int) { ++(*this); }

    decltype(auto) operator*() const noexcept { return coro->value(); }
    auto operator->() const noexcept { return std::addressof(coro->value()); }

    friend bool operator==(const coroutine_iterator &lhs, const coroutine_iterator_end &rhs) {
        return lhs.coro->done();
    }

    friend bool operator==(const coroutine_iterator_end &lhs, const coroutine_iterator &rhs) {
        return rhs.coro->done();
    }

private:
    std::add_pointer_t<coroutine_type> coro;
};
}// namespace coro
