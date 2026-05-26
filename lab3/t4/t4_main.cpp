#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include "server4.hpp"

int main() {
    const unsigned int available_cores = std::thread::hardware_concurrency();
    const unsigned short listening_port = 12345;

    try {
        boost::asio::io_context event_loop;

        FactorialServer main_server(event_loop, listening_port, available_cores);

        std::vector<std::thread> worker_threads;
        for (unsigned int idx = 0; idx < available_cores; ++idx) {
            worker_threads.emplace_back([&event_loop]() {
                event_loop.run();
            });
        }

        std::cout << "Server started on port " << listening_port
                  << " (threads: " << available_cores << ")\n"
                  << "Press ENTER to shutdown...\n";

        std::cin.get();

        std::cout << "Shutting down server...\n";
        main_server.shutdown();

        for (auto& worker : worker_threads)
            worker.join();

        std::cout << "Server terminated successfully.\n";
    } catch (const std::exception& error) {
        std::cerr << "Fatal error occurred" << std::endl;
        return 1;
    }
    return 0;
}
