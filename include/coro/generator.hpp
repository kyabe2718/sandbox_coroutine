#pragma once

#include "coro_base.hpp"

#include <iostream>

namespace coro {

template<typename T>
struct generator;

namespace detail {
template<typename T>
struct generator_promise_type final : promise_base<T> {
    static_assert(!std::is_same_v<T, void>);

    using base_type = promise_base<T>;
    using base_type::base_type;

    generator<T> get_return_object() noexcept;

    auto initial_suspend() noexcept { return std::suspend_always{}; }
    auto final_suspend() noexcept { return std::suspend_always{}; }

    std::suspend_always yield_value(auto &&value) noexcept {
        this->template set_value(std::forward<decltype(value)>(value));
        return {};
    }

    void return_void() {}

    auto awaitable_transform(auto) = delete;

    friend struct generator<T>;
};
}// namespace detail

template<typename T>
struct generator final : coroutine_base<detail::generator_promise_type<T>> {
    static_assert(!std::is_same_v<T, void>);

    using promise_type = detail::generator_promise_type<T>;
    using base_type = coroutine_base<promise_type>;
    using handle_type = typename base_type::handle_type;

    using base_type::base_type;
    friend promise_type;

    decltype(auto) value() const noexcept { return this->promise().value(); }

    void move_next() {
        base_type::resume();
        if (base_type::done())
            this->promise().rethrow_if_exception();
    }

    [[nodiscard]] coroutine_iterator<generator> begin() {
        // because generator_promise::initial_suspend returns suspend_always
        move_next();
        return coroutine_iterator<generator>(*this);
    }

    [[nodiscard]] coroutine_iterator_end end() { return {}; }

    decltype(auto) operator()() {
        move_next();
        return value();
    }

    friend generator &operator>>(generator &gen, T &value) {
        if (gen.done())
            throw std::runtime_error{"This coroutine is already done."};
        value = gen();
        return gen;
    }

private:
    using base_type::resume;
};

namespace detail {
template<typename T>
generator<T> generator_promise_type<T>::get_return_object() noexcept {
    static_assert(Promise<typename generator<T>::promise_type>);
    using handle_type = typename generator<T>::handle_type;
    return generator<T>(handle_type::from_promise(*this));
}
}// namespace detail
}// namespace coro
