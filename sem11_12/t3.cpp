#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>

using namespace boost::asio::experimental::awaitable_operators;

class FinancialAccount {
private:
    int current_balance_;
    boost::asio::strand<boost::asio::io_context::executor_type> operation_strand_;

public:
    FinancialAccount(boost::asio::io_context& event_loop) : current_balance_(0), operation_strand_(boost::asio::make_strand(event_loop.get_executor())) {}

    boost::asio::awaitable<void> add_funds(int amount) {
        co_await boost::asio::post(operation_strand_, boost::asio::use_awaitable);

        current_balance_ += amount;
        co_return;
    }

    boost::asio::awaitable<void> remove_funds(int amount) {
        co_await boost::asio::post(operation_strand_, boost::asio::use_awaitable);

        if (current_balance_ < amount) {
            throw std::invalid_argument("Insufficient funds");
        }
        current_balance_ -= amount;
        co_return;
    }

    boost::asio::awaitable<int> retrieve_balance() {
        co_await boost::asio::post(operation_strand_, boost::asio::use_awaitable);
        co_return current_balance_;
    }
}; 

boost::asio::awaitable<void> perform_transactions(FinancialAccount& user_account, std::atomic<int>& total_credits, std::atomic<int>& total_debits) {
    std::random_device random_source;
    std::mt19937 random_engine(random_source());
    std::uniform_int_distribution<> transaction_amount(1, 1000);
    std::uniform_int_distribution<> operation_type(0, 1);

    int local_credits = 0;
    int local_debits = 0;

    for (int iteration = 0; iteration < 20; ++iteration) {
        int amount = transaction_amount(random_engine);
        bool is_credit = operation_type(random_engine) == 0;

        if (is_credit) {
            co_await user_account.add_funds(amount);
            local_credits += amount;
        } else {
            try {
                co_await user_account.remove_funds(amount);
                local_debits += amount;
            } catch (const std::invalid_argument& error) {}
        }
        
        boost::asio::steady_timer delay_timer(co_await boost::asio::this_coro::executor);
        delay_timer.expires_after(std::chrono::microseconds(50));
        co_await delay_timer.async_wait(boost::asio::use_awaitable);
    }

    total_credits += local_credits;
    total_debits += local_debits;
}

int main() {
    try {
        boost::asio::io_context event_loop;

        FinancialAccount main_account(event_loop);

        std::atomic<int> total_deposited_amount(0);
        std::atomic<int> total_withdrawn_amount(0);
        std::atomic<int> remaining_operations(100);

        for (int client_index = 0; client_index < 100; ++client_index) {
            boost::asio::co_spawn(event_loop, perform_transactions(main_account, total_deposited_amount, total_withdrawn_amount),
                [&remaining_operations](std::exception_ptr exception_ptr) {
                    if (exception_ptr) {
                        try {
                            std::rethrow_exception(exception_ptr);
                        } catch (const std::exception& error) {
                            std::cerr << "Coroutine execution error: " << error.what() << std::endl;
                        }
                    }
                    --remaining_operations;
                });
        }

        std::vector<std::thread> worker_threads;
        for (int thread_id = 0; thread_id < 4; ++thread_id) {
            worker_threads.emplace_back([&event_loop]() {
                event_loop.run();
            });
        }

        while (remaining_operations > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        event_loop.stop();

        for (auto& active_thread : worker_threads) {
            active_thread.join();
        }

        int final_balance_value = 0;
        boost::asio::co_spawn(event_loop, main_account.retrieve_balance(),
            [&final_balance_value](std::exception_ptr, int balance_amount) {
                if (balance_amount) {
                    final_balance_value = balance_amount;
                }
            });

        event_loop.restart();
        event_loop.run();

        int anticipated_balance = total_deposited_amount.load() - total_withdrawn_amount.load();
        std::cout << "Banking simulation completed." << std::endl;
        std::cout << "Total Credits: " << total_deposited_amount.load() << std::endl;
        std::cout << "Total Debits: " << total_withdrawn_amount.load() << std::endl;
        std::cout << "Expected Balance: " << anticipated_balance << std::endl;
        std::cout << "Actual Balance: " << final_balance_value << std::endl;

        if (final_balance_value == anticipated_balance) {
            std::cout << "Verification PASSED: Account balance is consistent." << std::endl;
        } else {
            std::cout << "Verification FAILED: Account balance inconsistent." << std::endl;
        }
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
