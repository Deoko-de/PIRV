#include "client4.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

using boost::asio::ip::tcp;

namespace {
    bool is_valid_positive_integer(const std::string& text) {
        if (text.empty()) return false;
        if (!std::all_of(text.begin(), text.end(), ::isdigit))
            return false;
        if (text.size() > 1 && text[0] == '0')
            return false;
        return true;
    }
}

void start_client_connection() {
    try {
        boost::asio::io_context event_loop;
        tcp::socket client_socket(event_loop);
        tcp::resolver address_lookup(event_loop);
        auto server_endpoints = address_lookup.resolve("127.0.0.1", "12345");
        boost::asio::connect(client_socket, server_endpoints);

        std::cout << "Connected to server.\n"
                  << "Enter a number to compute factorial (or 'q' to quit):\n";

        while (true) {
            std::string user_input;
            std::getline(std::cin, user_input);

            if (user_input == "q" || user_input == "Q") break;

            if (!is_valid_positive_integer(user_input)) {
                std::cout << "Error: Please enter a non-negative integer "
                             "(no spaces, no leading zeros).\n";
                continue;
            }

            std::string outgoing_request = user_input + "\n";
            boost::asio::write(client_socket, boost::asio::buffer(outgoing_request));

            boost::asio::streambuf incoming_data;
            boost::asio::read_until(client_socket, incoming_data, '\n');
            std::istream input_stream(&incoming_data);
            std::string server_response;
            std::getline(input_stream, server_response);
            std::cout << "Server reply: " << server_response << std::endl;
        }
        
        client_socket.close();
        std::cout << "Connection closed.\n";
        
    } catch (const std::exception& error) {
        std::cerr << "Client error occurred" << std::endl;
    }
}

int main() {
    start_client_connection();
    return 0;
}
