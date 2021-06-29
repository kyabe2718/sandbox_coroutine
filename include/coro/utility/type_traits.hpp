#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace coro {

template<typename T, typename... Ts>
concept same_as_any_of = (std::same_as<T, Ts> || ...);

template<typename T, typename... Ts>
concept convertible_to_any_of = (std::is_convertible_v<T, Ts> || ...);

namespace detail {
template<typename P>
concept found_return_void = requires(P p) {
    p.return_void();
};

template<typename P>
concept found_return_value = requires(P p) {
    typename P::value_type;
    p.return_value(std::declval<typename P::value_type>());
};

template<typename Awaitable>
concept member_co_await = requires() { std::declval<Awaitable>().operator co_await(); };

template<typename Awaitable>
concept non_member_co_await = requires() { operator co_await(std::declval<Awaitable>()); };

template<typename Awaitable>
decltype(auto) get_awaiter(Awaitable &&awaitable) { return std::forward<Awaitable>(awaitable); }

template<member_co_await Awaitable>
auto get_awaiter(Awaitable &&awaitable) { return std::forward<Awaitable>(awaitable).operator co_await(); }

template<non_member_co_await Awaitable>
auto get_awaiter(Awaitable &&awaitable) { return operator co_await(std::forward<Awaitable>(awaitable)); }

}// namespace detail

template<typename T, typename = void>
struct awaitable_traits {};

template<typename Awaitable>
struct awaitable_traits<Awaitable, std::void_t<decltype(detail::get_awaiter(std::declval<Awaitable>()))>> {
    using awaiter_type = decltype(detail::get_awaiter(std::declval<Awaitable>()));
    using await_result_type = decltype(std::declval<awaiter_type>().await_resume());
};

}// namespace coro