#pragma once

namespace coro {

namespace detail {

template<typename T>
struct sync_wait_task {
};

}// namespace detail

template<typename Awaitable>
decltype(auto) sync_wait(Awaitable &&awaitable) {
    //
}

}// namespace coro