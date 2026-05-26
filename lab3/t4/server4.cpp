#include "server4.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cctype>

using namespace std::chrono_literals;

ClientSession::ClientSession(tcp::socket client_socket,
                 boost::asio::strand<boost::asio::io_context::executor_type> execution_strand,
                 std::vector<std::string>& event_log,
                 boost::asio::thread_pool& worker_pool)
    : connection_(std::move(client_socket)),
      strand_(std::move(execution_strand)),
      log_(event_log),
      worker_group_(worker_pool),
      inactivity_timer_(connection_.get_executor()) {}

void ClientSession::begin() {
    activate_timeout();
    initiate_reception();
}

void ClientSession::activate_timeout() {
    inactivity_timer_.expires_after(10s);
    auto owner = shared_from_this();
    inactivity_timer_.async_wait(boost::asio::bind_executor(strand_,
        [this, owner](boost::system::error_code error_flag) {
            if (!error_flag) {
                std::cout << "Idle timeout reached, closing connection.\n";
                boost::system::error_code ignored_error;
                connection_.shutdown(tcp::socket::shutdown_both, ignored_error);
                connection_.close(ignored_error);
                is_terminated_ = true;
            }
        }));
}

void ClientSession::disable_timeout() {
    inactivity_timer_.cancel();
}

void ClientSession::initiate_reception() {
    auto owner = shared_from_this();
    connection_.async_read_some(boost::asio::buffer(raw_buffer_, max_buffer_size),
        boost::asio::bind_executor(strand_,
            [this, owner](boost::system::error_code error_flag, std::size_t bytes_read) {
                if (error_flag) {
                    if (is_terminated_) return;
                    
                    if (error_flag == boost::asio::error::eof)
                        std::cerr << "Client disconnected.\n";
                    else if (error_flag == boost::asio::error::operation_aborted)
                        ;
                    else
                        std::cerr << "Read error. Connection lost.\n";
                    disable_timeout();
                    return;
                }
                disable_timeout();
                std::string incoming_data(raw_buffer_, bytes_read);
                while (!incoming_data.empty() &&
                       (incoming_data.back() == '\n' || incoming_data.back() == '\r'))
                    incoming_data.pop_back();
                handle_request(incoming_data);
            }));
}

void ClientSession::handle_request(const std::string& incoming_data) {
    if (incoming_data.empty() || !std::all_of(incoming_data.begin(), incoming_data.end(), ::isdigit)) {
        std::string error_message = "Error: Please enter a non-negative integer.\n";
        boost::asio::post(strand_, [this, owner = shared_from_this(), error_message]() {
            transmit_response(error_message);
        });
        return;
    }

    unsigned long numeric_value = std::stoul(incoming_data);

    if (numeric_value > 20) {
        std::string error_message = "Error: Supported range is 0 to 20.\n";
        boost::asio::post(strand_, [this, owner = shared_from_this(), error_message]() {
            transmit_response(error_message);
        });
        return;
    }

    auto owner = shared_from_this();
    boost::asio::post(worker_group_, [this, owner, numeric_value]() {
        unsigned long long computed_result = 1;
        for (unsigned long i = 2; i <= numeric_value; ++i)
            computed_result *= i;

        std::string reply = "Factorial " + std::to_string(numeric_value) +
                               " = " + std::to_string(computed_result) + "\n";

        boost::asio::post(strand_, [this, owner, reply]() {
            log_.push_back(reply);
            std::cout << "[LOG] " << reply;
            transmit_response(reply);
        });
    });
}

void ClientSession::transmit_response(const std::string& reply_message) {
    if (is_terminated_) return;
    auto owner = shared_from_this();
    boost::asio::async_write(connection_, boost::asio::buffer(reply_message),
        boost::asio::bind_executor(strand_,
            [this, owner](boost::system::error_code error_flag, std::size_t) {
                if (error_flag) {
                    std::cerr << "Transmission error" << "\n";
                    return;
                }
                if (!is_terminated_) {
                    activate_timeout();
                    initiate_reception();
                }
            }));
}

FactorialServer::FactorialServer(boost::asio::io_context& event_loop, unsigned short listening_port, int thread_count)
    : connection_acceptor_(event_loop, tcp::endpoint(tcp::v4(), listening_port)),
      execution_strand_(event_loop.get_executor()),
      worker_group_(thread_count),
      event_loop_(event_loop) {
    start_accepting();
}

void FactorialServer::start_accepting() {
    connection_acceptor_.async_accept(
        [this](boost::system::error_code error_flag, tcp::socket client_socket) {
            if (!error_flag) {
                std::make_shared<ClientSession>(std::move(client_socket),
                                          execution_strand_, transaction_log_, worker_group_)->begin();
            }
            if (connection_acceptor_.is_open())
                start_accepting();
        });
}

void FactorialServer::shutdown() {
    connection_acceptor_.close();
    worker_group_.join();
    event_loop_.stop();
}
