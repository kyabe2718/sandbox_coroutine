#include <coro/sync_wait.hpp>
#include <coro/task.hpp>

int main() {
    auto task = []() -> coro::task<> {
        std::cout << "hoge" << std::endl;
        co_return;
    };

    coro::sync_wait([&]() -> coro::task<> {
        std::cout << "fuga" << std::endl;
        co_await task();
        std::cout << "piyo" << std::endl;
        co_return;
    }());
}