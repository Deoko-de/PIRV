#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>

using boost::asio::ip::tcp;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    ClientSession(tcp::socket socket,
                  boost::asio::strand<boost::asio::io_context::executor_type> strand,
                  std::vector<std::string>& log,
                  boost::asio::thread_pool& pool);
    void begin();

private:
    void initiate_reception();
    void transmit_response(const std::string& msg);
    void handle_request(const std::string& request);
    void activate_timeout();
    void disable_timeout();

    tcp::socket connection_;
    boost::asio::strand<boost::asio::io_context::executor_type> execution_strand_;
    std::vector<std::string>& event_log_;
    boost::asio::thread_pool& worker_group_;
    boost::asio::steady_timer inactivity_timer_;
    static constexpr std::size_t max_buffer_size = 1024;
    char raw_buffer_[max_buffer_size];
    bool is_terminated_ = false;
};

class FactorialServer {
public:
    FactorialServer(boost::asio::io_context& io, unsigned short port, int threads);
    void shutdown();

private:
    void start_accepting();

    tcp::acceptor connection_acceptor_;
    boost::asio::strand<boost::asio::io_context::executor_type> execution_strand_;
    std::vector<std::string> transaction_log_;
    boost::asio::thread_pool worker_group_;
    boost::asio::io_context& event_loop_;
};
