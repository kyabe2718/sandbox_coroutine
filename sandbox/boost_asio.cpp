#include <boost/asio.hpp>
#include <chrono>
#include <iostream>

namespace asio = boost::asio;
using namespace std::chrono_literals;

int main() {
    asio::io_context ioc;
    ioc.post([&] {
        std::this_thread::sleep_for(1s);
        std::cout << std::this_thread::get_id() << ": " << __PRETTY_FUNCTION__ << std::endl;

        ioc.post([] {
            std::this_thread::sleep_for(1s);
            std::cout << std::this_thread::get_id() << ": " << __PRETTY_FUNCTION__ << std::endl;
        });

        ioc.post([] {
            std::this_thread::sleep_for(1s);
            std::cout << std::this_thread::get_id() << ": " << __PRETTY_FUNCTION__ << std::endl;
        });
    });

    std::thread thread1{[&] {
        ioc.run();
    }};

    ioc.run();
    thread1.join();
}