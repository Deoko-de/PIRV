#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;
        
        tcp::socket client_socket(io_context);
        tcp::endpoint server_endpoint(boost::asio::ip::make_address("127.0.0.1"), 12345);
        client_socket.connect(server_endpoint);

        std::cout << "Enter message to send: ";
        std::string outgoing_data;
        std::getline(std::cin, outgoing_data);
        outgoing_data += "\n";

        boost::asio::write(client_socket, boost::asio::buffer(outgoing_data));

        boost::asio::streambuf incoming_buffer;
        boost::asio::read_until(client_socket, incoming_buffer, '\n');

        std::istream input_stream(&incoming_buffer);
        std::string server_reply;
        std::getline(input_stream, server_reply);

        std::cout << "Server response: " << server_reply << std::endl;

    } catch (std::exception& error) {
        std::cerr << "Client error: " << error.what() << std::endl;
    }
    return 0;
}
