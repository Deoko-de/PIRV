#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Client : public std::enable_shared_from_this<Client> {
public:
    Client(boost::asio::io_context& event_loop, const tcp::resolver::results_type& server_endpoints)
        : connection_(event_loop) {
        establish_connection(server_endpoints);
    }

    void transmit_data(const std::string& payload) {
        auto owner(shared_from_this());
        boost::asio::async_write(connection_, boost::asio::buffer(payload),
            [this, owner](boost::system::error_code error_flag, size_t /*bytes_sent*/) {
                if (!error_flag) {
                    std::cout << "Data transmitted successfully." << std::endl;
                    initiate_reception();
                } else {
                    std::cout << "Transmission error: " << error_flag.message() << std::endl;
                }
            });
    }

private:
    void establish_connection(const tcp::resolver::results_type& server_endpoints) {
        auto owner(shared_from_this());
        boost::asio::async_connect(connection_, server_endpoints,
            [this, owner](boost::system::error_code error_flag, tcp::endpoint) {
                if (!error_flag) {
                    std::cout << "Client connected to server." << std::endl;
                } else {
                    std::cout << "Connection error: " << error_flag.message() << std::endl;
                }
            });
    }

    void initiate_reception() {
        auto owner(shared_from_this());
        connection_.async_read_some(boost::asio::buffer(storage_buffer_, buffer_capacity),
            [this, owner](boost::system::error_code error_flag, size_t bytes_received) {
                if (!error_flag) {
                    std::cout << "Received from server: " << std::string(storage_buffer_, bytes_received) << std::endl;
                } else {
                    std::cout << "Reception error: " << error_flag.message() << std::endl;
                }
            });
    }

    tcp::socket connection_;
    enum { buffer_capacity = 1024 };
    char storage_buffer_[buffer_capacity];
};

int main() {
    try {
        boost::asio::io_context event_loop;

        tcp::resolver address_lookup(event_loop);
        auto target_endpoints = address_lookup.resolve("127.0.0.1", "12345");

        auto network_client = std::make_shared<Client>(event_loop, target_endpoints);

        std::thread worker_thread([&event_loop]() { event_loop.run(); });

        std::string user_input;
        while (true) {
            std::cout << "Enter coordinates (e.g., 55.75,37.61): ";
            std::getline(std::cin, user_input);

            if (user_input.empty()) {
                break;
            }

            network_client->transmit_data(user_input);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        event_loop.stop();
        worker_thread.join();
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
