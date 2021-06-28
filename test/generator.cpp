#undef NDEBUG

#include <cassert>

#include <coro/generator.hpp>
#include <iostream>
#include <vector>

using namespace coro;

#define assert_eq(lhs, rhs)                                                         \
    do {                                                                            \
        auto l = (lhs);                                                             \
        auto r = (rhs);                                                             \
        if (l != r) {                                                               \
            std::cerr << "lhs: " << l << ", rhs: " << r << std::endl;               \
            __assert_fail(#lhs " == " #rhs, __FILE__, __LINE__, __ASSERT_FUNCTION); \
        }                                                                           \
    } while (0)

int main() {

    try {
        auto gen = []() -> generator<int> {
            co_yield 1;
            co_yield 2;
            throw std::runtime_error{"hogehoge"};
            co_yield 3;
        }();
        gen.move_next();
        assert_eq(gen.value(), 1);
        gen.move_next();
        assert_eq(gen.value(), 2);
        gen.move_next();
        assert(false);
    } catch (std::runtime_error &e) {
    } catch (...) {
        assert(false);
    }

    try {
        auto gen = []() -> generator<int> {
            co_yield 1;
            co_yield 2;
            throw std::runtime_error{"hogehoge"};
            co_yield 3;
        }();
        assert_eq(gen(), 1);
        assert_eq(gen(), 2);
        assert((gen(), false));
    } catch (std::runtime_error &e) {
    } catch (...) {
        assert(false);
    }

    {
        std::vector<int> vec{1, 2, 3, 4, 5};
        auto gen = [&]() -> generator<int> {
            for (int e : vec)
                co_yield e;
        }();
        assert(std::equal(vec.begin(), vec.end(), gen.begin()));
    }

    {
        std::vector<int> vec{1, 2, 3, 4, 5};
        auto gen = [&]() -> generator<int> {
            for (int e : vec)
                co_yield e;
        }();
        for (int e : vec) {
            int v;
            gen >> v;
            assert_eq(e, v);
        }
    }

    return 0;
}
