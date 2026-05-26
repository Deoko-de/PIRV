#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int total_message_counter = 0;
std::shared_ptr<boost::asio::strand<boost::asio::io_context::executor_type>> execution_strand;

class ClientHandler : public std::enable_shared_from_this<ClientHandler> {
public:
    ClientHandler(tcp::socket client_socket) 
    : connection_(std::move(client_socket)) {}

    void begin_handling() {
        initiate_reception();
    }

private:
    void initiate_reception() {
        auto owner(shared_from_this());
        connection_.async_read_some(boost::asio::buffer(storage_buffer_, buffer_limit),
            boost::asio::bind_executor(*execution_strand, 
            [this, owner](boost::system::error_code error_flag, size_t bytes_received) {
                if (!error_flag) {
                    std::cout << "Message received: " << std::string(storage_buffer_, bytes_received) << std::endl;
                    ++total_message_counter;
                    std::cout << "Total messages count: " << total_message_counter << std::endl;

                    transmit_response(bytes_received);
                }
            })
        );
    }

    void transmit_response(size_t bytes_received) {
        auto owner(shared_from_this());
        boost::asio::async_write(connection_, boost::asio::buffer(storage_buffer_, bytes_received),
            boost::asio::bind_executor(*execution_strand, 
            [this, owner](boost::system::error_code error_flag, size_t /*bytes_sent*/){
                if (!error_flag) {
                    initiate_reception();
                }
            }));
    }

    tcp::socket connection_;
    enum { buffer_limit = 1024 };
    char storage_buffer_[buffer_limit];
};

class NetworkServer {
public:
    NetworkServer(boost::asio::io_context& event_loop, short listening_port) : 
        connection_acceptor_(event_loop, tcp::endpoint(tcp::v4(), listening_port)) {
        start_accepting();
    }

private:
    void start_accepting() {
        connection_acceptor_.async_accept(
            boost::asio::make_strand(connection_acceptor_.get_executor()),
            [this](boost::system::error_code error_flag, tcp::socket client_connection) {
                if (!error_flag) {
                    std::make_shared<ClientHandler>(std::move(client_connection))->begin_handling();
                }
                start_accepting();
            }
        );
    }

    tcp::acceptor connection_acceptor_;
};

int main() {
    try {
        boost::asio::io_context event_loop;
        execution_strand = std::make_shared<boost::asio::strand<boost::asio::io_context::executor_type>>(event_loop.get_executor());
    
        NetworkServer main_server(event_loop, 12345);

        std::vector<std::thread> worker_threads;
        int thread_pool_size = 4;

        for (int thread_idx = 0; thread_idx < thread_pool_size; ++thread_idx) {
            worker_threads.emplace_back([&event_loop] {
                event_loop.run();
            });
        }

        for (auto& active_thread : worker_threads) {
            active_thread.join();
        }
    } catch (const std::exception& error) {
        std::cerr << "System error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
