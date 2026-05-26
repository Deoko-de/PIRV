#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <memory>

using boost::asio::ip::tcp;

class ClientHandler : public std::enable_shared_from_this<ClientHandler> {
public:
    ClientHandler(tcp::socket client_socket) : connection_(std::move(client_socket)) {}

    void begin_handling() {
        initiate_reception();
    }

private:
    void initiate_reception() {
        auto owner(shared_from_this());
        connection_.async_read_some(boost::asio::buffer(incoming_data_),
            [this, owner](boost::system::error_code error_flag, std::size_t bytes_received) {
                if (!error_flag) {
                    std::string raw_input(incoming_data_, bytes_received);
                    handle_client_input(raw_input);
                }
            });
    }

    void handle_client_input(const std::string& raw_input) {
        auto owner(shared_from_this());
        boost::asio::post(connection_.get_executor(), [this, owner, raw_input]() {
            std::istringstream input_parser(raw_input);
            std::vector<int> numeric_values;
            int current_value;
            
            while (input_parser >> current_value) {
                numeric_values.push_back(current_value);
            }

            std::string server_reply;
            if (numeric_values.empty()) {
                server_reply = "ERROR: number list is empty\n";
            } else {
                auto highest = std::max_element(numeric_values.begin(), numeric_values.end());
                server_reply = "Maximum value: " + std::to_string(*highest) + "\n";
            }
            
            transmit_response(server_reply);
        });
    }

    void transmit_response(const std::string& server_reply) {
        auto owner(shared_from_this());
        boost::asio::async_write(connection_, boost::asio::buffer(server_reply),
            [this, owner](boost::system::error_code error_flag, std::size_t) {
                if (!error_flag) {
                    initiate_reception();
                }
            });
    }

    tcp::socket connection_;
    char incoming_data_[1024];
};

class NetworkAcceptor {
public:
    NetworkAcceptor(boost::asio::io_context& io_context, short listening_port)
        : connection_acceptor_(io_context, tcp::endpoint(tcp::v4(), listening_port)) {
        start_accepting();
    }

private:
    void start_accepting() {
        connection_acceptor_.async_accept(
            [this](boost::system::error_code error_flag, tcp::socket client_connection) {
                if (!error_flag) {
                    std::make_shared<ClientHandler>(std::move(client_connection))->begin_handling();
                }
                start_accepting();
            });
    }
    
    tcp::acceptor connection_acceptor_;
};

int main() {
    try {
        boost::asio::io_context event_loop;
        NetworkAcceptor server(event_loop, 12345);
        std::cout << "Async server running on port 12345...\n";
        event_loop.run();
    } catch (std::exception& exception_obj) {
        std::cerr << "Fatal error: " << exception_obj.what() << std::endl;
    }
    return 0;
}
