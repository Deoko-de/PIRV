#include <iostream>
#include <string>
#include <variant>
#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>

using namespace boost::asio::experimental::awaitable_operators;
using boost::asio::ip::tcp;

boost::asio::awaitable<std::string> receive_data(tcp::socket& connection, std::string identifier) {
    char buffer_storage[1024];
    auto [error_flag, bytes_retrieved] = co_await connection.async_read_some(boost::asio::buffer(buffer_storage), boost::asio::as_tuple(boost::asio::use_awaitable));

    if (error_flag) {
        co_return "Failed to read from " + identifier + ": " + error_flag.message();
    }

    co_return std::string(buffer_storage, bytes_retrieved);
}

boost::asio::awaitable<void> data_multiplexer(tcp::socket& first_connection, tcp::socket& second_connection) {
    for (;;) {
        auto operation_result = co_await (receive_data(first_connection, "Channel A") || receive_data(second_connection, "Channel B"));

        std::visit([](auto& response_data) {
            std::cout << response_data << std::endl;
        }, operation_result);
    }
}

int main() {
    try {
        boost::asio::io_context event_loop;

        tcp::acceptor listener_first(event_loop, tcp::endpoint(tcp::v4(), 12345));
        tcp::acceptor listener_second(event_loop, tcp::endpoint(tcp::v4(), 12346));

        std::cout << "Awaiting clients on ports 12345 and 12346..." << std::endl;
        boost::asio::co_spawn(event_loop, [&]() -> boost::asio::awaitable<void> {
            tcp::socket client_first = co_await listener_first.async_accept(boost::asio::use_awaitable);
            tcp::socket client_second = co_await listener_second.async_accept(boost::asio::use_awaitable);
            std::cout << "Clients established on both endpoints." << std::endl;
            co_await data_multiplexer(client_first, client_second);
        }, boost::asio::detached);

        event_loop.run();
    } catch (const std::exception& error) {
        std::cerr << "System error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
