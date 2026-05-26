#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <sstream>

using boost::asio::ip::tcp;

class ClientConnection : public std::enable_shared_from_this<ClientConnection> {
public:
    ClientConnection(tcp::socket client_socket) 
        : connection_(std::move(client_socket)), delay_timer_(connection_.get_executor()) {}

    void begin_session() {
        start_reading();
    }

private:
    void start_reading() {
        auto owner(shared_from_this());
        boost::asio::async_read_until(connection_, incoming_data_, '\n',
            [this, owner](boost::system::error_code error_flag, std::size_t bytes_received) {
                if (!error_flag) {
                    std::istream input_stream(&incoming_data_);
                    std::string raw_command;
                    std::getline(input_stream, raw_command);
                    handle_command(raw_command);
                }
            });
    }

    void handle_command(const std::string& raw_command) {
        std::istringstream command_parser(raw_command);
        std::string command_type;
        double duration = -1;

        if (!(command_parser >> command_type >> duration) || command_type != "timer") {
            transmit_error("ERROR: Invalid input format. Usage: timer <number>\n");
            return;
        }

        if (duration <= 0) {
            transmit_error("ERROR: Duration must be a positive number\n");
            return;
        }

        int wait_seconds = static_cast<int>(duration);
        std::string confirmation = "Timer started for " + std::to_string(wait_seconds) + " sec\n";
        auto owner(shared_from_this());
        
        boost::asio::async_write(connection_, boost::asio::buffer(confirmation),
            [this, owner, wait_seconds](boost::system::error_code error_flag, std::size_t) {
                if (!error_flag) {
                    delay_timer_.expires_after(boost::asio::chrono::seconds(wait_seconds));
                    delay_timer_.async_wait([this, owner](boost::system::error_code error_flag) {
                        if (!error_flag) {
                            notify_completion();
                        }
                    });
                }
            });
    }

    void transmit_error(const std::string& error_message) {
        auto owner(shared_from_this());
        boost::asio::async_write(connection_, boost::asio::buffer(error_message),
            [this, owner](boost::system::error_code error_flag, std::size_t) {
                if (!error_flag) {
                    start_reading();
                }
            });
    }

    void notify_completion() {
        auto owner(shared_from_this());
        std::string completion_msg = "Completed!\n";
        boost::asio::async_write(connection_, boost::asio::buffer(completion_msg),
            [this, owner](boost::system::error_code error_flag, std::size_t) {
                if (!error_flag) {
                    start_reading();
                }
            });
    }

    tcp::socket connection_;
    boost::asio::steady_timer delay_timer_;
    boost::asio::streambuf incoming_data_;
};

class AsyncServer {
public:
    AsyncServer(boost::asio::io_context& event_loop, short listening_port)
        : connection_acceptor_(event_loop, tcp::endpoint(tcp::v4(), listening_port)) {
        start_accepting();
    }

private:
    void start_accepting() {
        connection_acceptor_.async_accept(
            [this](boost::system::error_code error_flag, tcp::socket client_socket) {
                if (!error_flag) {
                    std::make_shared<ClientConnection>(std::move(client_socket))->begin_session();
                }
                start_accepting();
            });
    }
    
    tcp::acceptor connection_acceptor_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        AsyncServer server_instance(io_context, 12345);
        std::cout << "Timer server running on port 12345..." << std::endl;
        io_context.run();
    } catch (std::exception& exception_obj) {
        std::cerr << "System error: " << exception_obj.what() << std::endl;
    }
    return 0;
}
