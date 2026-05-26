#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;
        
        tcp::acceptor listener(io_context, tcp::endpoint(tcp::v4(), 12345)); 
        
        std::cout << "Server started on port 12345..." << std::endl;

        for (;;) {
            tcp::socket client_connection(io_context);
            listener.accept(client_connection);

            boost::asio::streambuf data_buffer;
            boost::asio::read_until(client_connection, data_buffer, '\n');

            std::istream input_stream(&data_buffer);
            std::string received_text;
            std::getline(input_stream, received_text);

            size_t text_length = received_text.length();
            std::string converted_text = received_text;
            std::transform(converted_text.begin(), converted_text.end(), converted_text.begin(), 
                           [](unsigned char ch) { return std::toupper(ch); });

            std::string reply_message = std::to_string(text_length) + ": " + converted_text + "\n";
            
            boost::asio::write(client_connection, boost::asio::buffer(reply_message)); 
        }
    } catch (std::exception& error) {
        std::cerr << "Server error: " << error.what() << std::endl;
    }
    return 0;
}
