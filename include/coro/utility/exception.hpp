#pragma once

#include <stdexcept>

namespace coro {

struct broken_promise : std::logic_error {
    broken_promise()
        : std::logic_error("broken promise") {}
};

}// namespace coro