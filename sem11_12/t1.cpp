#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

boost::asio::awaitable<void> handle_client_communication(tcp::socket client_socket) {
    char message_buffer[1024];
    try {
        for (;;) {
            auto [error_code, bytes_read] = co_await client_socket.async_read_some(
                boost::asio::buffer(message_buffer), boost::asio::as_tuple(boost::asio::use_awaitable)
            );

            if (error_code == boost::asio::error::eof) {
                std::cout << "Client terminated connection." << std::endl;
                break;
            }

            if (error_code) {
                throw boost::system::system_error(error_code);
            }

            co_await boost::asio::async_write(client_socket, boost::asio::buffer(message_buffer, bytes_read), boost::asio::use_awaitable);
        }
    } catch (const std::exception& exception_obj) {
        std::cerr << "Session processing error: " << exception_obj.what() << std::endl;
    }
}

boost::asio::awaitable<void> launch_server_endpoint(tcp::acceptor connection_acceptor) {
    for (;;) {
        tcp::socket client_connection = co_await connection_acceptor.async_accept(boost::asio::use_awaitable);
        std::cout << "New client connected successfully." << std::endl;

        auto current_executor = co_await boost::asio::this_coro::executor;

        boost::asio::co_spawn(current_executor, handle_client_communication(std::move(client_connection)), boost::asio::detached);
    }
}

int main() {
    try {
        boost::asio::io_context event_loop;

        tcp::acceptor listener(event_loop, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Echo service running on port 12345..." << std::endl;

        boost::asio::co_spawn(event_loop, launch_server_endpoint(std::move(listener)), boost::asio::detached);
        event_loop.run();
    } catch (const std::exception& exception_obj) {
        std::cerr << "Fatal server error: " << exception_obj.what() << std::endl;
        return 1;
    }

    return 0;
}
