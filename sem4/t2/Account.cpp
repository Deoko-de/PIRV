#include <mutex>
#include <condition_variable>
#include <iostream>

template<typename NumericType>
class BankAccount {

private:
    NumericType funds;
    std::mutex operation_lock;
    std::condition_variable fund_available;

public:

    BankAccount(NumericType initial_funds = 0) : funds(initial_funds) {}

    NumericType check_balance() {
        std::lock_guard<std::mutex> guard(operation_lock);
        return funds;
    }

    void add_funds(NumericType value) {
        std::unique_lock<std::mutex> locker(operation_lock);

        funds += value;

        fund_available.notify_all();
    }

    void withdraw_funds(NumericType value) {

        std::unique_lock<std::mutex> locker(operation_lock);

        fund_available.wait(locker, [&]() {
            return funds >= value;
        });

        funds -= value;
    }

    std::mutex& retrieve_mutex() {
        return operation_lock;
    }

    std::condition_variable& retrieve_condition_var() {
        return fund_available;
    }
};
