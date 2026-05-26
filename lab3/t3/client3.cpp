#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::socket client_socket(io_context);
        tcp::resolver address_resolver(io_context);
        boost::asio::connect(client_socket, address_resolver.resolve("127.0.0.1", "12345"));

        std::cout << "Enter command (e.g., timer 5): ";
        std::string user_input;
        std::getline(std::cin, user_input);
        user_input += "\n";

        boost::asio::write(client_socket, boost::asio::buffer(user_input));

        for (int reply_count = 0; reply_count < 2; ++reply_count) {
            boost::asio::streambuf data_buffer;
            boost::asio::read_until(client_socket, data_buffer, '\n');
            std::istream input_stream(&data_buffer);
            std::string server_reply;
            std::getline(input_stream, server_reply);
            std::cout << "Server: " << server_reply << std::endl;
        }

    } catch (std::exception& error) {
        std::cerr << "Client error: " << error.what() << "\n";
    }
    return 0;
}
